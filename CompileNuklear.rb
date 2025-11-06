require 'fileutils'
require 'etc'
require 'open3'
require 'rbconfig'

NUKLEAR_MAKEFILE = File.expand_path('ThirdParty/Nuklear/Makefile', __dir__)

def usage_and_exit
  puts "Usage: ruby #{File.basename(__FILE__)}"
  exit 1
end

unless File.exist?(NUKLEAR_MAKEFILE)
  puts "❌ Nuklear Makefile not found at #{NUKLEAR_MAKEFILE}"
  exit 1
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
  false
end

host_os = RbConfig::CONFIG['host_os']
is_windows = host_os =~ /mswin|mingw|cygwin/

build_commands = if is_windows
                   [
                     "make -f \"#{NUKLEAR_MAKEFILE}\"",
                     "mingw32-make -f \"#{NUKLEAR_MAKEFILE}\"" # Fall back for if Make.exe isint found
                   ]
                 else
                   ["make -f \"#{NUKLEAR_MAKEFILE}\""]
                 end

success = false
build_commands.each do |cmd|
  success = run_streaming(cmd)
  break if success
  puts "⚠️  Command failed: #{cmd}" unless success
end

unless success
  puts "❌ Failed to build Nuklear using all known commands."
  exit 1
end

puts "✅ Nuklear compiled successfully!"