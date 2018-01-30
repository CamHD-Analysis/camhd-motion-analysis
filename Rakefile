
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
