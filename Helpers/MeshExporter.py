# Run in blender to get your mesh and provide a name for it at the OUTPATH variable
import bpy
import struct
import os

# ---------- CONFIG ----------
OUTPATH = bpy.path.abspath("//MeshNameHere.mesh")
FORMAT_VERSION = 1
# ---------- END CONFIG ----------

def write_string(f, s):
    b = s.encode('utf-8')
    f.write(struct.pack('<H', len(b)))
    f.write(b)

def export_all_meshes(outpath):
    depsgraph = bpy.context.evaluated_depsgraph_get()
    meshes_exported = 0

    objs = [o for o in bpy.data.objects if o.type == 'MESH']

    with open(outpath, 'wb') as f:
        f.write(b'BMESH\x00')
        f.write(struct.pack('<B', FORMAT_VERSION))
        f.write(struct.pack('<I', len(objs)))

        for obj in objs:
            # Evaluate object to get modifiers/applied geometry
            eval_obj = obj.evaluated_get(depsgraph)
            mesh = eval_obj.to_mesh()
            if mesh is None:
                print(f"Skipping {obj.name}: no mesh produced by evaluation.")
                continue

            mesh.calc_loop_triangles()
            mesh.calc_normals_split()

            uv_layer = None
            if mesh.uv_layers.active:
                uv_layer = mesh.uv_layers.active.data

            loop_to_vtx_index = {}
            out_vertices = []
            out_indices = []

            for tri in mesh.loop_triangles:
                tri_indices = []
                for li, loop_index in enumerate(tri.loops):
                    if loop_index in loop_to_vtx_index:
                        idx = loop_to_vtx_index[loop_index]
                    else:
                        loop = mesh.loops[loop_index]
                        vert = mesh.vertices[loop.vertex_index]

                        co = vert.co[:]
                        normal = loop.normal[:]
                        if uv_layer is not None:
                            uv = uv_layer[loop_index].uv[:]  # (u, v)
                        else:
                            uv = (0.0, 0.0)
                        idx = len(out_vertices)
                        out_vertices.append((co, normal, uv))
                        loop_to_vtx_index[loop_index] = idx
                    tri_indices.append(idx)

                if len(tri_indices) != 3:
                    continue
                out_indices.extend(tri_indices)

            write_string(f, obj.name)

            mat = obj.matrix_world
            mat_flat = [float(x) for row in mat for x in row]
            f.write(struct.pack('<16f', *mat_flat))

            vertex_count = len(out_vertices)
            index_count = len(out_indices)

            f.write(struct.pack('<I', vertex_count))
            f.write(struct.pack('<I', index_count))

            for (co, normal, uv) in out_vertices:
                f.write(struct.pack('<3f', *co))
                f.write(struct.pack('<3f', *normal))
                f.write(struct.pack('<2f', *uv))

            # Write indices as uint32
            for idx in out_indices:
                f.write(struct.pack('<I', idx))

            meshes_exported += 1

            try:
                eval_obj.to_mesh_clear()
            except AttributeError:
                try:
                    bpy.data.meshes.remove(mesh)
                except Exception:
                    pass

    print(f"Exported {meshes_exported} mesh object(s) to: {outpath}")


if __name__ == "__main__":
    odir = os.path.dirname(OUTPATH)
    if odir and not os.path.exists(odir):
        os.makedirs(odir, exist_ok=True)

    export_all_meshes(OUTPATH)
