module.exports = grunt => {
    grunt.initConfig({
        concat: {
            options: {
                sourceMap: true
            },
            "js": {
                src: ["build/web/*tests.js"],
                dest: "build/test/app.concat.js"
            },
        },

        uglify: {
            my_target: {
                options: {
                    sourceMap: false,
                    mangle: false,
                },
                files: {
                    "build/test/app.min.js" : ["build/test/app.concat.js"],
                    "build/test/appWASM.js" : ["build/test/appWASM.js"]
                }
            }
        },

        exec: {
            build: "source ~/emsdk/emsdk_env.sh & echo Building... & emcmake cmake -S . -B build && (cd build && emmake make -j 20)"
        },

        watch: {
            cpp: {
                files: ["test/*.cpp", "test/*.h"],
                tasks: ["exec:build", "uglify"]
            },
            js: {
                files: ["build/web/*.js"],
                tasks: ["concat", "uglify"]
            }
        }
    })

    grunt.loadNpmTasks("grunt-contrib-watch")
    grunt.loadNpmTasks('grunt-contrib-concat')
    grunt.loadNpmTasks('grunt-contrib-uglify-es')
    grunt.loadNpmTasks("grunt-exec")

    grunt.registerTask("default", ["watch"])
}
