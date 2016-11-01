
task :default => "debug:test"

@cmake_opts = ['-D BUILD_UNIT_TESTS:BOOL=True']
load 'config.rb' if FileTest::exists? 'config.rb'

['Debug','Release'].each { |build_type|
  build_namespace = build_type.downcase.to_sym
  namespace build_namespace do
    build_dir = ENV['BUILD_DIR'] || "build-#{build_type}"

    task :env do
      sh "echo $OpenCV_DIR"
    end

    task :build do
      unless FileTest::directory? build_dir
        FileUtils::mkdir build_dir
        sh "cd %s && cmake -D CMAKE_BUILD_TYPE:STRING=\"%s\" %s  .." % [build_dir, build_type, @cmake_opts.join(' ')]
      end

      if !FileTest::exists? build_dir + "/g3log/src/g3log-build/libg3logger.a"
        sh "cd %s && make deps" % build_dir
      end

      sh "cd %s && make" % build_dir
    end

    task :test => :build do
      sh "cd %s && make unit_test" % build_dir
    end

    task :distclean do
      sh "rm -rf #{build_dir}"
    end
  end

  task :distclean => "#{build_namespace}:distclean"
}

namespace :dependencies do
  task :linux do
    sh "sudo apt-get install -y cmake opencv-dev"
  end

  task :osx do
    sh "brew update"
    sh "brew tap homebrew/science"
    sh "brew install cmake homebrew/science/opencv"
  end
end


#
# BUILD_DIR = "build"
#
#
# task :default => :build
#
# task :build do
#   unless FileTest::exists?( BUILD_DIR + "/g3log/src/g3log-build/libg3logger.a" )
#     sh "cd %s && make deps" % BUILD_DIR
#   end
#
#   sh "cd %s && make" % BUILD_DIR
# end
#
# task :test do
#   sh "cd %s && make unit_test" % BUILD_DIR
# end
#
# task :bootstrap do
# opencv_24_dir = "/opt/opencv-2.4/share/OpenCV"
#
#   FileUtils::mkdir BUILD_DIR unless FileTest::directory? BUILD_DIR
#   sh "cd %s && OpenCV_DIR=#{opencv_24_dir} cmake -D CMAKE_BUILD_TYPE:STRING=\"Debug\" " \
#       "-D LOCAL_LIBACTIVE_OBJECT:FILEPATH=../libactive_object " \
#       "-D LOCAL_LIBLOGGER:FILEPATH=../liblogger " \
#       ".." % BUILD_DIR
# end
