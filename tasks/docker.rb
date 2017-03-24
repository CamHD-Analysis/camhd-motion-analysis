require 'rake'
require 'pathname'

#
# Tasks for creating a Docker instance for testing
#

class DockerTasks

  include Rake::DSL

  def initialize( opts = {})

    @builds = opts[:builds] || %w( debug release )

    @root_dir = Pathname.new(__FILE__).parent.parent
    @docker_path = @root_dir.join('docker/test/')

    define_tasks
  end


  def docker_image
    "camhd_motion_tracking:test"
  end

  def docker_run_opts
    %W( --rm
        -v #{@root_dir}:/opt/camhd_motion_tracking
        #{docker_image})
  end

  def in_docker
    chdir @docker_path do
      yield
    end
  end

  def docker_run *args
    in_docker do
      sh *(['docker', 'run'] + docker_run_opts + args)
    end
  end

  def define_tasks


    namespace :docker do

      desc "Build test docker image."
      task :build_image do
        in_docker {
          sh "docker build -t #{docker_image} ."
        }
      end

      @builds.each do |build|

        namespace build do

          desc "Make deps for #{build} in Docker"
          task :deps do
            docker_run "#{build}:deps"
          end

          desc "Make #{build} in Docker"
          task :build do
            docker_run "#{build}:build"
          end

          desc "Run #{build} tests in Docker"
          task :test do
            docker_run "#{build}:test"
          end

          task :clean do
            docker_run "#{build}:clean"
          end

        end
      end

      desc "Open console in Docker"
      task :console do
        in_docker {
          args = %w(docker run -ti --entrypoint /bin/bash) + docker_run_opts
          sh *args
        }
      end
    end

  end


end
