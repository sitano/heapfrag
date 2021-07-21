require "bundler/gem_tasks"
require "rspec/core/rake_task"

RSpec::Core::RakeTask.new(:spec)

require "rake/extensiontask"

task :build => :compile

Rake::ExtensionTask.new("heapfrag") do |ext|
  ext.lib_dir = "lib/heapfrag"
end

task :default => [:clobber, :compile, :spec]
