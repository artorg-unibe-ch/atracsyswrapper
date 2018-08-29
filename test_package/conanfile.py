from conans import ConanFile, CMake
import os

class OpenIGTLinkTestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin")

    def test(self):
        self.run("cd bin && .%satracsyswrapper_test" % os.sep)