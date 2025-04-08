from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain, CMake, cmake_layout

class MyProjectConan(ConanFile):
    name = "imguiWinApp"
    version = "1.0.0"

    settings = "os", "arch", "compiler", "build_type"

    requires = [
        "imgui/1.91.4-docking",
        "spdlog/1.15.1",
        "openssl/3.3.2",
        "boost/1.86.0",
        "nlohmann_json/3.11.3",
    ]

    generators = ("CMakeDeps", "CMakeToolchain")

    default_options = {
        "imgui/*:shared": False,
        "spdlog/*:shared": False,
        
        "openssl/*:shared": False,     

        "boost/*:header_only": False,
        "boost/*:without_atomic": False,
        "boost/*:without_chrono": False,
        "boost/*:without_context": False,
        "boost/*:without_coroutine": True,
        "boost/*:without_date_time": False,
        "boost/*:without_filesystem": False,
        "boost/*:without_graph": True,
        "boost/*:without_iostreams": True,
        "boost/*:without_json": True,
        "boost/*:without_locale": True,
        "boost/*:without_log": True,
        "boost/*:without_math": True,
        "boost/*:without_prg_exec_monitor": True,
        "boost/*:without_program_options": True,
        "boost/*:without_random": True,
        "boost/*:without_regex": True,
        "boost/*:without_serialization": True,
        "boost/*:without_stacktrace": True,
        "boost/*:without_test": True,
        "boost/*:without_timer": True,
        "boost/*:without_type_erasure": True,
        "boost/*:without_wave": True,
        "boost/*:without_container": False,
        "boost/*:without_system": False,
        "boost/*:without_thread": False,
        "boost/*:without_fiber": True,
        "boost/*:without_beast": False,
        "boost/*:without_asio": False,
    }

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    # Add packaging if needed:
    # def package(self):
    #     cmake = CMake(self)
    #     cmake.install()
    #
    # def package_info(self):
    #     self.cpp_info.libs = ["..."]
