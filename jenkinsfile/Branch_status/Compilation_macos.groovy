pipeline {
    agent { label 'osx && arm64' }
    options { 
        buildDiscarder(logRotator(numToKeepStr: '20', daysToKeepStr: '15'))
        gitLabConnection('GitLabConnectionJenkins')
    }

    stages {
        stage('Checkout SDK'){
            steps {
                dir('src/MEGASync/mega'){
                    checkout([
                        $class: 'GitSCM', 
                        branches: [[name: "${SDK_BRANCH}"]],
                        userRemoteConfigs: [[ url: "${env.GIT_URL_SDK}", credentialsId: "12492eb8-0278-4402-98f0-4412abfb65c1" ]],
                        extensions: [
                            [$class: "UserIdentity",name: "jenkins", email: "jenkins@jenkins"]
                            ]
                    ])
                }
                script { 
                    desktop_sources_workspace = WORKSPACE
                }
            }
        }
        stage('Build desktop'){
			environment{
                PATH = "/usr/local/bin:${env.PATH}"
                BUILD_DIR = "build_dir"
                MEGAQTPATH= "${env.HOME}/Qt-build/5.15.13/5.15.13/arm64"
                VCPKGPATH = "${env.HOME}/jenkins/vcpkg"
			}
            steps{
                dir(desktop_sources_workspace){
                    sh "mkdir build_dir"
                    sh "cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_VERBOSE_MAKEFILE=1 -DVCPKG_ROOT=${VCPKGPATH} ${BUILD_OPTIONS} -DCMAKE_PREFIX_PATH=${MEGAQTPATH} -S ${desktop_sources_workspace} -B ${desktop_sources_workspace}/${BUILD_DIR}"
                    sh "cmake --build ${desktop_sources_workspace}/${BUILD_DIR} --target MEGAsync -j2"
                    sh "cmake --build ${desktop_sources_workspace}/${BUILD_DIR} --target MEGAupdater -j2"
                    sh "xcodebuild clean build CODE_SIGN_IDENTITY=\"-\" CODE_SIGNING_REQUIRED=NO -jobs 1 -configuration Debug -target MEGAShellExtFinder -project src/MEGAShellExtFinder/MEGAFinderSync.xcodeproj/"
                }
            }
        }
    }
    post{
        always {
            deleteDir()
        }
        // success {
        // }
        // failure {
        // }
        // aborted {
        // }
    }
}
