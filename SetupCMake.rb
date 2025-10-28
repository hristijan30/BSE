require 'fileutils'
require 'etc'
require 'open3'

GENERATOR_MAP = {
  'gcc'   => 'Unix Makefiles',
  'mingw' => 'MinGW Makefiles',
  'msvc'  => 'Visual Studio 17 2022',
  'vs'    => 'Visual Studio 17 2022',
  'ninja' => 'Ninja',
  'xcode' => 'Xcode'
}.freeze

def usage_and_exit
  puts "Usage: ruby #{File.basename(__FILE__)} <compiler> [Config]"
  puts "Available compilers: #{GENERATOR_MAP.keys.map(&:upcase).join(', ')}"
  puts "Config (optional): Debug, Release, RelWithDebInfo, MinSizeRel (default: Release)"
  exit 1
end

if ARGV.empty?
  usage_and_exit
end

compiler_key = ARGV[0].to_s.downcase
config = (ARGV[1] || 'Release').to_s

unless GENERATOR_MAP.key?(compiler_key)
  puts "❌ Unknown compiler/generator: '#{ARGV[0]}'"
  usage_and_exit
end

generator = GENERATOR_MAP[compiler_key]
project_root = Dir.pwd
build_dir = File.join(project_root, 'build')

unless Dir.exist?(build_dir)
  puts "📁 Creating build directory at #{build_dir}"
  begin
    FileUtils.mkdir_p(build_dir)
  rescue SystemCallError => e
    puts "❌ Failed to create build directory: #{e.message}"
    exit 1
  end
end

def run_streaming(cmd, env = {})
  puts "▶ #{cmd}"
  Open3.popen2e(env, cmd) do |stdin, stdout_err, wait_thr|
    stdout_err.each { |line| print line }
    exit_status = wait_thr.value
    return exit_status.success?
  end
rescue Errno::ENOENT => e
  puts "❌ Command not found: #{e.message}"
  return false
end

cmake_config_cmd = %(cmake -S "#{project_root}" -B "#{build_dir}" -G "#{generator}")
puts "⚙️  Configuring project with CMake generator: #{generator}"
unless run_streaming(cmake_config_cmd)
  puts "❌ CMake configuration failed. Make sure CMake and the requested generator are installed."
  exit 1
end

cores = begin
  Etc.nprocessors
rescue StandardError
  2
end
puts "ℹ️  Using #{cores} parallel build job(s)."

env = { 'CMAKE_BUILD_PARALLEL_LEVEL' => cores.to_s }

multi_config_generator = generator.match?(/Visual Studio|Xcode|Multi-Config/i)

build_cmd_base = if multi_config_generator
                   %(cmake --build "#{build_dir}" --config #{config})
                 else
                   %(cmake --build "#{build_dir}")
                 end

build_cmd_parallel = "#{build_cmd_base} --parallel #{cores}"
puts "🔨 Building project (preferred method using CMake --parallel)..."
if run_streaming(build_cmd_parallel, env)
  puts "✅ Build completed (used --parallel). Output in #{build_dir}"
  exit 0
end

if generator.match?(/Visual Studio/i)
  puts "⚠️  --parallel failed. Trying MSBuild parallel flag fallback (/m)..."
  msbuild_fallback = "#{build_cmd_base} -- /m:#{cores}"
  if run_streaming(msbuild_fallback, env)
    puts "✅ Build completed with MSBuild /m. Output in #{build_dir}"
    exit 0
  else
    puts "❌ MSBuild fallback also failed."
    exit 1
  end
else
  puts "⚠️  --parallel failed for generator '#{generator}'. Trying build without --parallel (but with CMAKE_BUILD_PARALLEL_LEVEL env)..."
  if run_streaming(build_cmd_base, env)
    puts "✅ Build completed (without explicit parallel flag). Output in #{build_dir}"
    exit 0
  else
    puts "❌ Build failed."
    exit 1
  end
end
