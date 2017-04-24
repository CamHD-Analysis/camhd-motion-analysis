from conans import ConanFile, CMake

class LibMotionTracking(ConanFile):
  name = "libvideoio"
  version = "0.1"
  settings = "os", "compiler", "build_type", "arch"
  generators = "cmake"
  options = {"opencv_dir": "ANY",
            "build_parallel": [True, False],
            "with_openmp": [True, False]}
  default_options = "opencv_dir=''", "build_parallel=True", "with_openmp=False"
  requires =  "TCLAP/master@jmmut/testing", \
              "g3log/master@amarburg/testing"

  def config(self):
    #self.options[""].opencv_dir = self.options.opencv_dir

    if self.scope.dev and self.scope.build_tests:
      self.requires( "gtest/1.8.0@lasote/stable" )
      self.options["gtest"].shared = False

  def imports(self):
    self.copy("*.dll", dst="bin", src="bin") # From bin to bin
    self.copy("*.dylib*", dst="bin", src="lib") # From lib to bin

  def build(self):
    cmake = CMake(self.settings)
    cmake_opts = ""
    cmake_opts += "-DOpenCV_DIR=%s " % (self.options.opencv_dir) if self.options.opencv_dir else ""
    cmake_opts += "-DBUILD_UNIT_TESTS=True " if self.scope.dev and self.scope.build_tests else ""
    cmake_opts += "-DUSE_OPENMP=%s " % (self.options.with_openmp)

    build_opts = "-j" if self.options.build_parallel else ""

    self.run('cmake "%s" %s %s' % (self.conanfile_directory, cmake.command_line, cmake_opts))
    self.run('cmake --build . %s -- %s' % (cmake.build_config, build_opts))
    if self.scope.dev and self.scope.build_tests:
      self.run('make unit_test')
