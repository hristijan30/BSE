import bpy
import struct
import io
import zlib

bl_info = {
    "name": "Export BMESH (.mesh)",
    "author": "KikoMira",
    "version": (1, 2),zzz
    "blender": (2, 80, 0),
    "location": "File > Export > BMESH (.mesh)",
    "category": "Import-Export",
}

def get_evaluated_mesh(obj, depsgraph):
    """Try several to_mesh() signatures across Blender versions."""
    eval_obj = obj.evaluated_get(depsgraph)
    for attempt in (
        lambda o: o.to_mesh(),
        lambda o: o.to_mesh(depsgraph),
        lambda o: o.to_mesh(preserve_all_data_layers=True, depsgraph=depsgraph),
    ):
        try:
            mesh = attempt(eval_obj)
            if mesh is not None:
                return eval_obj, mesh
        except TypeError:
            continue
        except Exception:
            continue
    try:
        mesh = eval_obj.to_mesh()
        return eval_obj, mesh
    except Exception:
        return eval_obj, None

def safe_free_eval_mesh(eval_obj, mesh):
    """Cleanup evaluated mesh in a way compatible across versions."""
    try:
        eval_obj.to_mesh_clear()
    except Exception:
        try:
            if mesh is not None:
                mesh.free()
        except Exception:
            pass

def write_mesh(filepath, obj, auto_unwrap=False, compress=False):
    depsgraph = bpy.context.evaluated_depsgraph_get()
    eval_obj, mesh = get_evaluated_mesh(obj, depsgraph)
    if mesh is None:
        print("Failed to obtain evaluated mesh for object:", obj.name)
        return

    if hasattr(mesh, "calc_loop_triangles"):
        mesh.calc_loop_triangles()

    if hasattr(mesh, "calc_normals"):
        mesh.calc_normals()

    if auto_unwrap and (not mesh.uv_layers):
        try:
            bpy.ops.object.mode_set(mode='OBJECT')
            pass
        except Exception:
            pass

    uv_layer = mesh.uv_layers.active
    has_uvs = uv_layer is not None

    vertex_map = {}
    positions = []
    normals = []
    uvs = []
    indices = []
    next_idx = 0

    if not hasattr(mesh, "loop_triangles"):
        print("Mesh has no loop_triangles; aborting export.")
        safe_free_eval_mesh(eval_obj, mesh)
        return

    for tri in mesh.loop_triangles:
        tri_ids = []
        for loop_index in tri.loops:
            vert_index = mesh.loops[loop_index].vertex_index
            vert = mesh.vertices[vert_index]
            pos = vert.co

            n = None
            try:
                loop = mesh.loops[loop_index]
                if hasattr(loop, "normal"):
                    n = loop.normal.copy()
            except Exception:
                n = None

            if n is None:
                try:
                    if hasattr(tri, "normal"):
                        n = tri.normal.copy()
                except Exception:
                    n = None

            if n is None:
                n = vert.normal.copy()

            if has_uvs:
                try:
                    uv = uv_layer.data[loop_index].uv
                    uv_key = (float(uv.x), float(uv.y))
                except Exception:
                    uv_key = (0.0, 0.0)
            else:
                uv_key = (0.0, 0.0)

            key = (
                round(pos.x, 6), round(pos.y, 6), round(pos.z, 6),
                round(n.x, 6), round(n.y, 6), round(n.z, 6),
                uv_key[0], uv_key[1]
            )

            if key not in vertex_map:
                vertex_map[key] = next_idx
                positions.append((pos.x, pos.y, pos.z))
                normals.append((n.x, n.y, n.z))
                uvs.append(uv_key)
                next_idx += 1

            tri_ids.append(vertex_map[key])

        indices.extend(tri_ids)

    vertex_count = len(positions)
    index_count = len(indices)

    buf = io.BytesIO()

    buf.write(struct.pack("I", 1))

    name_bytes = obj.name.encode('utf-8')
    buf.write(struct.pack("H", len(name_bytes)))
    buf.write(name_bytes)

    mat = obj.matrix_world
    for r in range(4):
        for c in range(4):
            buf.write(struct.pack("f", mat[r][c]))

    buf.write(struct.pack("I", vertex_count))
    buf.write(struct.pack("I", index_count))

    for i in range(vertex_count):
        px, py, pz = positions[i]
        nx, ny, nz = normals[i]
        ux, uy = uvs[i]
        buf.write(struct.pack("3f", px, py, pz))
        buf.write(struct.pack("3f", nx, ny, nz))
        buf.write(struct.pack("2f", ux, uy))

    if index_count > 0:
        buf.write(struct.pack(f"{index_count}I", *indices))

    payload = buf.getvalue()

    with open(filepath, "wb") as f:
        f.write(b"BMESH")
        f.write(struct.pack("B", 2))

        if compress:
            flags = 1
        else:
            flags = 0

        f.write(struct.pack("B", flags))

        if compress:
            compressed = zlib.compress(payload)
            f.write(struct.pack("I", len(compressed)))
            f.write(struct.pack("I", len(payload)))
            f.write(compressed)
        else:
            f.write(payload)

    safe_free_eval_mesh(eval_obj, mesh)
    print(f"Exported '{filepath}' -> {vertex_count} verts, {index_count} indices (UVs: {has_uvs})")

class ExportBMESH_NoSplit(bpy.types.Operator): # Split normal error fixed now
    """Export BMESH (.mesh) - safe no-split-normals"""
    bl_idname = "export_scene.bmesh_nosplit"
    bl_label = "Export BMESH (.mesh)"

    filepath: bpy.props.StringProperty(subtype="FILE_PATH")
    auto_unwrap: bpy.props.BoolProperty(
        name="Auto Unwrap (not for evaluated objects)",
        description="Attempt to create UVs if none exist (best to unwrap in Edit Mode before export)",
        default=False,
    )
    compress: bpy.props.BoolProperty(
        name="Compress",
        description="Compress mesh payload with zlib to reduce file size",
        default=False,
    )

    def execute(self, context):
        obj = context.active_object
        if not obj or obj.type != 'MESH':
            self.report({'ERROR'}, "Select a mesh object")
            return {'CANCELLED'}
        write_mesh(self.filepath, obj, auto_unwrap=self.auto_unwrap, compress=self.compress)
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}

def menu_func_export(self, context):
    self.layout.operator(ExportBMESH_NoSplit.bl_idname, text="BMESH (.mesh) - safe")

def register():
    bpy.utils.register_class(ExportBMESH_NoSplit)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)

def unregister():
    bpy.utils.unregister_class(ExportBMESH_NoSplit)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)

if __name__ == "__main__":
    register()
