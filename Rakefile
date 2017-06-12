
$:.unshift File.dirname(__FILE__) + "/tasks"
require 'docker'

task :default => "release:test"

@conan_opts = {  build_parallel: 'False' }
@conan_settings = {}
@conan_scopes = { build_tests: 'True' }
@conan_build = "outdated"
load 'config.rb' if FileTest.readable? 'config.rb'


## TODO. Add task to check for git submodules...

buildTypes =  ['Debug','Release']
buildTypes.each { |build_type|
  namespace build_type.downcase.to_sym do
    build_dir = ENV['BUILD_DIR'] || "build-#{build_type}"

    @conan_settings[:build_type] = build_type
    conan_opts = @conan_opts.each_pair.map { |key,val| "-o %s=%s" % [key,val] } +
                @conan_settings.each_pair.map { |key,val| "-s %s=%s" % [key,val] } +
                @conan_scopes.each_pair.map { |key,val| "--scope %s=%s" % [key,val] }

    task :build do
      FileUtils::mkdir build_dir unless FileTest::directory? build_dir
      sh "conan source ."
      chdir build_dir do
        sh "conan install %s .. --build=%s" % [conan_opts.join(' '), @conan_build]
        sh "conan build .."
      end
    end

    task :test => :build do
      #
    end
  end
}


#DockerTasks.new

namespace :docker do
  namespace :deploy do
    task :base do
        sh "docker build --tag camhd_motion_analysis_base:latest --tag camhd_motion_analysis_base:#{`git rev-parse --short HEAD`.chomp} docker/base"
    end

    task :build => :base do
      chdir "docker/deploy" do
        sh "docker build --tag camhd_motion_analysis:latest --tag camhd_motion_analysis:#{`git rev-parse --short HEAD`.chomp} docker-deploy"
      end
    end

    task :test do
      sh "docker run camhd_motion_analysis:latest --help"
    end

    task :push_gcr => :test do
      registry_url = "us.gcr.io/camhd-image-statistics/camhd_motion_analysis"
      sh "docker tag camhd_motion_analysis:latest #{registry_url}"
      sh "gcloud docker -- push #{registry_url}"
    end
  end
end


namespace :rq do

  namespace :test do
    redis_url = "redis://192.168.13.108:6379/"

    task :build do
      sh "docker build --tag camhd_motion_analysis_rq_worker:test " \
          "--file docker/rq_worker/Dockerfile_test ."
    end

    task :launch do
      sh "docker run \
          --volume /home/aaron/canine/camhd_analysis/CamHD_motion_metadata:/output/CamHD_motion_metadata camhd_motion_analysis_rq_worker:test --redis-url #{redis_url} --log INFO"
    end

    task :inject do
      chdir "python" do
        sh "python3 ./rq_client.py --redis-url #{redis_url} " \
            " --threads 16 " \
            " --output-dir /output/CamHD_motion_metadata /RS03ASHS/PN03B/06-CAMHDA301/2016/01/01/CAMHDA301-20160101T180000Z.mov"
      end
    end

    namespace :swarm do

      task :push do
        sh "docker tag camhd_motion_analysis_rq_worker:test 127.0.0.1:5000/camhd_motion_analysis_rq_worker:test"
        sh "docker push 127.0.0.1:5000/camhd_motion_analysis_rq_worker:test"
      end

      task :launch do
         sh "docker service create" \
            " --name test_worker" \
            " --network lazycache_nocache_default" \
            " --mount type=bind,source=/auto/canine/aaron/camhd_analysis/CamHD_motion_metadata,destination=/output/CamHD_motion_metadata "\
            " 127.0.0.1:5000/camhd_motion_analysis_rq_worker:test"\
            " --redis-url #{redis_url} --log INFO"
      end

      task :update do
        sh "docker service update --force test_worker"
      end

      task :inject do
        chdir "python" do
          sh "python3 ./rq_client.py --redis-url #{redis_url} " \
              " --threads 16 " \
              " --lazycache-url http://lazycache_nocache:8080/v1/org/oceanobservatories/rawdata/files/" \
              " --output-dir /output/CamHD_motion_metadata /RS03ASHS/PN03B/06-CAMHDA301/2016/01/01/CAMHDA301-20160101T090000Z.mov"
        end
      end
    end

  end

  task :base_image do
      chdir "docker/rq_worker/" do
      sh "docker build --tag camhd_motion_analysis_rq_worker_base:latest --tag camhd_motion_analysis_rq_worker_base:#{`git rev-parse --short HEAD`.chomp} --file Dockerfile_base ."
    end
  end

  namespace :prod do
    task :build => "rq:base_image" do
      chdir "docker/rq_worker/" do
        sh "git push"
        sh "docker build --no-cache "\
            " --tag camhd_motion_analysis_rq_worker:latest " \
            " --tag camhd_motion_analysis_rq_worker:#{`git rev-parse --short HEAD`.chomp} "\
            " --file Dockerfile_pristine ."
      end
    end

    task :push do
      sh "docker tag camhd_motion_analysis_rq_worker:latest amarburg/camhd_motion_analysis_rq_worker:latest"
      sh "docker push amarburg/camhd_motion_analysis_rq_worker:latest"
    end

    task :launch do
      sh "docker run --detach --env RQ_REDIS_URL=\"redis://ursine:6379/\" --volume /output/CamHD_motion_metadata:/home/aaron/canine/camhd_analysis/CamHD_motion_metadata/ camhd_motion_analysis_rq_worker:latest"
    end

    namespace :swarm do

      worker_name = "worker"

      task :launch do

        redis_url = ENV['REDIS_URL']
        throw "Please set environment variable REDIS_URL" unless redis_url

         sh "docker service create" \
            " --name #{worker_name}" \
            " --network lazycache_nocache_default" \
            " --mount type=bind,source=/auto/canine/aaron/camhd_analysis/CamHD_motion_metadata,destination=/output/CamHD_motion_metadata "\
            " amarburg/camhd_motion_analysis_rq_worker:latest"\
            " --redis-url #{redis_url} --log INFO"
      end

      task :update do
        sh "docker service update --force #{worker_name}"
      end

      task :inject do
        redis_url = ENV['REDIS_URL']
        throw "Please set environment variable REDIS_URL" unless redis_url

        chdir "python" do
          sh "python3 ./rq_client.py --redis-url #{redis_url} " \
              " --threads 16 " \
              " --lazycache-url http://lazycache_nocache:8080/v1/org/oceanobservatories/rawdata/files/" \
              " --output-dir /output/CamHD_motion_metadata"\
              " /RS03ASHS/PN03B/06-CAMHDA301/2016/01/01/CAMHDA301-20160101T060000Z.mov"
        end
      end
    end

  end

end


task :stitch do
  sh "build-Debug/bin/stitch --display --regions /home/aaron/workspace/camhd_analysis/CamHD_motion_metadata/RS03ASHS/PN03B/06-CAMHDA301/2016/01/01/CAMHDA301-20160101T000000Z_optical_flow_regions.json"
end


task :process_short do
  sh "build-Debug/bin/frame_stats  --display -o CAMHDA301-20160101T000000Z_short.json --start-at 5000 --stop-at 5050 --stride 10 /RS03ASHS/PN03B/06-CAMHDA301/2016/01/01/CAMHDA301-20160101T000000Z.mov
"
end

task :process_gpu do
  sh "build-Debug/bin/frame_stats --gpu -j 1 -o CAMHDA301-20160101T000000Z_gpu.json --start-at 5000 --stop-at 6000 --stride 10 /RS03ASHS/PN03B/06-CAMHDA301/2016/01/01/CAMHDA301-20160101T000000Z.mov
"
end

task :process do
  sh "build-Release/bin/frame_stats  -o CAMHDA301-20160101T000000Z.json --stride 10 /RS03ASHS/PN03B/06-CAMHDA301/2016/01/01/CAMHDA301-20160101T000000Z.mov
"
end



namespace :timelapse do
  task :get do
    sh "rm -f timelapse/*"
    sh "time build-Debug/bin/timelapse"
  end

  task :encode do
    sh "cat timelapse/*.png | ffmpeg -y -framerate 2 -f image2pipe -i -  -pix_fmt yuv420p timelapse.mov"
  end
end

namespace :sequence do
  task :get do
    sh "rm -f sequence/*"
    sh "time build-Debug/bin/sequence"
  end

  task :encode do
    sh "cat sequence/*.png | ffmpeg -y -framerate 3 -f image2pipe -i - -pix_fmt yuv420p sequence.mov"
  end
end


task :service do
  sh "docker service create -e RQ_REDIS_URL --mount type=bind,src=$METADATA_VOLUME,dst=/output/CamHD_motion_metadata  --replicas 1 --name worker amarburg/camhd_motion_analysis_rq_worker:latest"
end
