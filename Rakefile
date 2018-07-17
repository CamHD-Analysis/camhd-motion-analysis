
task :process_short do
  sh "./fips run frame_stats -o CAMHDA301-20160101T000000Z_short.json --frame 5000 /RS03ASHS/PN03B/06-CAMHDA301/2016/01/01/CAMHDA301-20160101T000000Z.mov
"
end

task :process_gpu do
  sh "./fips run frame_stats --gpu -o CAMHDA301-20160101T000000Z_gpu.json --start-at 5000 --stop-at 6000 --stride 10 /RS03ASHS/PN03B/06-CAMHDA301/2016/01/01/CAMHDA301-20160101T000000Z.mov
"
end

task :process do
  sh "./fips run frame_stats  -o CAMHDA301-20160101T000000Z.json --stride 10 /RS03ASHS/PN03B/06-CAMHDA301/2016/01/01/CAMHDA301-20160101T000000Z.mov
"
end

task :process_python do
  sh "python3 python/frame_stats.py  --start 5000 --stop 5050 --force --log INFO RS03ASHS/PN03B/06-CAMHDA301/2016/01/18/CAMHDA301-20160118T150000Z.mov"
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



### Deployment tasks

require 'bundler'
require 'dotenv'
require 'fileutils'
require 'date'

PUBLIC_LAZYCACHE_URL = "https://cache.camhd.science/v1/org/oceanobservatories/rawdata/files/"
LOCAL_LAZYCACHE_URL  = "http://lazycache:8080/v1/org/oceanobservatories/rawdata/files"

lazycache_name = "lazycache"
network_name = lazycache_name

lazycache_image_dockerhub = "amarburg/camhd_cache:latest"
#lazycache_image_gcr = "us.gcr.io/camhd-motion-analysis-swarm/lazycache_prod:latest"
lazycache_github = "github.com/amarburg/go-lazycache"
# = "#{ENV['GOPATH']}/src/github.com/amarburg/go-lazycache/deploy/docker/"

worker_image_base = "amarburg/camhd-worker-base"
worker_image      = "amarburg/camhd-worker"
#worker_image_gcr = "us.gcr.io/camhd-motion-analysis-swarm/camhd_motion_analysis_rq_worker:latest"


# instance_name   = "swarm-coreos-alpha-n1-highcpu-8"
# instance_version = "rev4"
# instance_fullname = [instance_name, instance_version].join('-')


def in_docker( &blk )
  chdir( "docker/", &blk )
end

namespace :network do
  task :create do
    docker *%W{ network create --attachable --driver=overlay #{network_name} }
  end
end

namespace :worker do

  task :prod => "worker:prod:build"

  namespace :prod do

    # Builds the "production" worker image.  This includes a pristine checkout of
    # camhd_motion_analysis from github
    task :build=> ["worker:base:build", "deploy/Dockerfile_prod"] do
        sh "docker", "build", "--no-cache",
                      "--tag", "#{worker_image}:latest",
                      "--tag", "#{worker_image}:#{`git rev-parse --short HEAD`.chomp}",
                      "--file", "deploy/Dockerfile_prod",
                      "."
    end

    task :run => :build do
      env_file = 'conf/prod.env'
      Dotenv.load(env_file)
      sh "docker", "run", *%W{--rm
                     --env-file #{env_file}
                     --network lazycache
                     --volume camhd_motion_metadata_by_nfs:/output/CamHD_motion_metadata
                     #{worker_image_dockerhub} --log INFO }
    end

    task :push do
      run "docker", "push", worker_image_dockerhub
      sh "gcloud docker -- push #{worker_image_gcr}"
    end

  end


  task :base => "worker:base:build"

  namespace :base do

    args = %W{ --tag #{worker_image_base}:latest
               --tag #{worker_image_base}:#{`git rev-parse --short HEAD`.chomp}
               --file deploy/Dockerfile_base .  }

    task :build => ["deploy/Dockerfile_base"] do
      sh "docker", "build", *args
    end

    task :force do
      sh "docker", "build", "--no-cache", *args
    end
  end


  task :test => "worker:test:build"

  namespace :test do

    desc "Builds a test docker image using a local copy of camhd_motion_analysis"
    task :build => ["worker:base:build", "deploy/Dockerfile_test"] do

      Dotenv.load('test.env')
      sh "docker", "build", "--tag", "#{worker_image}:test",
              "--file", "deploy/Dockerfile_test", "."
    end

    task :run => :build do
      sh "docker", "run",  *%W{run --rm
                  --env-file test.env
                  --volume /home/aaron/canine/camhd_analysis/CamHD_motion_metadata:/output/CamHD_motion_metadata
                  #{worker_image_test}  --log INFO }
    end


    desc "Run the test image using the production configuration"
    task :run_prod_env do
      Dotenv.load('conf/prod.env')
      sh "docker run --rm --env-file conf/prod.env "\
      " --network lazycache" \
      " --volume camhd_motion_metadata_by_nfs:/output/CamHD_motion_metadata "\
      " camhd_motion_analysis_rq_worker:test --log INFO"
    end

    desc "Inject a job using the test client"
    task :inject do
      #" --lazycache-url http://lazycache_nocache:8080/v1/org/oceanobservatories/rawdata/files" \

      Dotenv.load('conf/test.env')
      sh "docker run --rm --env-file conf/test.env "\
      " --entrypoint python3 "\
      " --network lazycache" \
      " --volume camhd_motion_metadata_by_nfs:/output/CamHD_motion_metadata" \
      " camhd_motion_analysis_rq_worker:test"\
      " /code/camhd_motion_analysis/python/rq_job_injector.py " \
      " --log INFO" \
      " --threads 16 " \
      " --output-dir /output/CamHD_motion_metadata"\
      " /RS03ASHS/PN03B/06-CAMHDA301/2016/02/01/"
    end

  end


end



  # ## Tasks specific to running on gcloud
  # namespace :gcloud do
  #
  #   task :lazycache do
  #     sh "gcloud docker --authorize-only"
  #     sh "docker service create --with-registry-auth "\
  #           " --name #{lazycache_name} "\
  #           "--constraint node.role!=manager "\
  #           "--network #{network_name} -p 8080 #{lazycache_image_gcr}"
  #     sh "docker service scale lazycache=8"
  #   end
  #
  #   task :redis do
  #     sh "docker run  --detach --env-file gcloud/prod.env -p 6379:6379 "\
  #           "--restart always "\
  #           "--name redis -v /home/amarburg/bitnami:/bitnami bitnami/redis:latest"
  #   end
  #
  #
  #   task :worker do
  #     sh "gcloud docker --authorize-only"
  #     sh "docker service create  --with-registry-auth "\
  #           "--env-file gcloud/prod.env --name worker "\
  #           "--constraint node.role!=manager --network #{network_name} "\
  #           "--mount type=volume,volume-opt=o=addr=swarm-manager,volume-opt=device=:/home/amarburg/CamHD_motion_metadata,volume-opt=type=nfs,source=camhd_motion_metadata_by_nfs,target=/output/CamHD_motion_metadata,volume-nocopy " \
  #           "#{worker_image_gcr} --log INFO"
  #     sh "docker service scale worker=16"
  #   end
  #
  #
  #   namespace :manager do
  #     task :ssh do
  #       sh "gcloud --project=camhd-motion-analysis-swarm compute ssh swarm-manager "
  #     end
  #
  #     task :create do
  #       sh "docker-machine --project=camhd-motion-analysis-swarm create swarm-manager -d google " \
  #       " --google-machine-type g1-small "\
  #       " --google-zone us-central1-a "\
  #       " --google-tags swarm-cluster "\
  #       " --google-project camhd-motion-analysis-swarm " \
  #       " --swarm-master " \
  #       " --google-tags redis-tcp-6379" \
  #       " --metadata-from-file startup-script=gcloud/swarm-manager-startup-script.sh"
  #     end
  #   end
  #
  #   ## Tasks for setting up the swarm
  #   namespace :instance_group do
  #
  #     task :create_template do
  #       sh "gcloud --project=camhd-motion-analysis-swarm compute instance-templates create #{instance_fullname} \
  #       --machine-type n1-highcpu-8 \
  #       --preemptible \
  #       --image-family coreos-alpha \
  #       --image-project coreos-cloud \
  #       --boot-disk-size 10GB \
  #       --metadata-from-file user-data=gcloud/swarm-worker-cloud-init"
  #     end
  #
  #     task :describe do
  #       sh "gcloud --project=camhd-motion-analysis-swarm compute instance-templates describe #{instance_fullname}"
  #     end
  #
  #     task :start do
  #       sh "gcloud --project=camhd-motion-analysis-swarm compute instance-groups managed create worker-alpha-swarm \
  #       --template #{instance_fullname} \
  #       --base-instance-name #{instance_name} \
  #       --size 1 \
  #       --zone us-central1-a"
  #     end
  #
  #   end
  #
  #   task :refresh => [:stop_cluster,
  #     "gcloud:instance_group:start",
  #     "gcloud:lazycache",
  #     "gcloud:refresh:scale_lazycache_and_wait",
  #     "gcloud:worker",
  #     "gcloud:refresh:scale_worker_and_wait"]
  #
  #
  #     task :stop_cluster do
  #       sh "docker service rm lazycache"
  #       sh "docker service rm worker"
  #
  #       sh "gcloud --project=camhd-motion-analysis-swarm compute instance-groups managed delete worker-alpha-swarm"
  #     end
  #
  #     # "Hidden" tasks to support cluster refresh
  #     namespace :refresh do
  #
  #       task :scale_lazycache_and_wait do
  #         sh "docker service scale lazycache=8"
  #         sh "sleep 120"   # TODO make this a closed loop on swarm status
  #       end
  #
  #       task :scale_worker_and_wait do
  #         sh "docker service scale worker=16"
  #       end
  #
  #     end

  #
  #   ## Gcloud-image inject jobs
  #   desc "Inject new jobs into the RQ queue; use the env variable INJECT_PATH"
  #   task :inject do
  #     do_inject(inject_path, network_name, worker_image_gcr, lazycache_name)
  #   end
  #
  #   inject_window = ENV["INJECT_WINDOW"].to_i || 1
  #
  #   desc "Inject recent jobs; set INJECT_WINDOW"
  #   task :inject_recent do
  #
  #     (Date.today-inject_window).upto( Date.today ) { |date|
  #
  #       path = "/RS03ASHS/PN03B/06-CAMHDA301/%04d/%02d/%02d/" % [date.year, date.month, date.mday]
  #       puts path
  #
  #       do_inject(path, network_name, worker_image_gcr, lazycache_name)
  #
  #     }
  #
  #   end
  #
  #
  #
  # end

namespace :local do

  ## Assumes you've done "docker-compose up"
  task :inject do
    sh "docker exec camhd-motion-analysis_camhd-worker_1 ./recent_injector.py  --days 1 --log INFO"
  end



end

  #
  #
  #   namespace :desktop do
  #
  #     desc "Launch lazycache on the desktop cluster"
  #     task :lazycache do
  #       sh "docker service create --name lazycache --network #{network_name} -p 8080 #{lazycache_image_dockerhub}"
  #     end
  #
  #     desc "Launch the optical flow worker on the desktop cluster"
  #     task :worker do
  #       sh "docker service create --env-file prod.env --name worker "\
  #       "--network #{network_name} "\
  #       "--mount type=volume,volume-opt=o=addr=192.168.13.110,volume-opt=device=:/mnt/zvol1/users/aaron/camhd_analysis/CamHD_motion_metadata/,volume-opt=type=nfs,source=camhd_motion_metadata_by_nfs,target=/output/CamHD_motion_metadata,volume-nocopy " \
  #       "#{worker_image_dockerhub} --log INFO"
  #     end
  #
  #   end
  #
  #
  # inject_path = ENV["INJECT_PATH"] || " /RS03ASHS/PN03B/06-CAMHDA301/2016/03/01/"
  #
  # def do_inject(inject_path, network, image, lazycache)
  #   Dotenv.load('conf/prod.env')
  #
  #   ## Use the public version
  #   #" --lazycache-url http://#{lazycache}:8080/v1/org/oceanobservatories/rawdata/files" \
  #
  #   docker_run *%W{--rm --env-file conf/prod.env
  #                --entrypoint python3
  #                --network #{network}
  #                --volume camhd_motion_metadata_by_nfs:/output/CamHD_motion_metadata
  #                #{image}
  #                /code/camhd_motion_analysis/python/rq_job_injector.py
  #                --threads 16
  #                --log INFO
  #                --output-dir /output/CamHD_motion_metadata
  #                --client-lazycache-url #{PUBLIC_LAZYCACHE_URL}
  #                --lazycache-url #{LOCAL_LAZYCACHE_URL}
  #                #{inject_path} }
  # end
  #
  # desc "Inject new jobs into the RQ queue; use the env variable INJECT_PATH"
  # task :inject do
  #   do_inject(inject_path, network_name, worker_image_dockerhub, lazycache_name)
  # end
  #
  # inject_window = ENV["INJECT_WINDOW"].to_i || 1
  #
  # desc "Inject recent jobs; set INJECT_WINDOW"
  # task :inject_recent do
  #
  #   (Date.today-inject_window).upto( Date.today ) { |date|
  #
  #     path = "/RS03ASHS/PN03B/06-CAMHDA301/%04d/%02d/%02d/" % [date.year, date.month, date.mday]
  #     puts path
  #
  #     do_inject(path, network_name, worker_image_dockerhub, lazycache_name)
  #
  #   }
  #
  # end
