pipeline {
    agent { label 'linux && amd64' }
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
				VCPKGPATH = "/opt/vcpkg"
				BUILD_DIR = "build_dir"
            }
            steps{
                dir("${desktop_sources_workspace}") {
                    sh "rm -rf ${BUILD_DIR}; mkdir ${BUILD_DIR}"
                    sh "cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DVCPKG_ROOT=${VCPKGPATH} ${BUILD_OPTIONS} -DCMAKE_VERBOSE_MAKEFILE=ON -S ${desktop_sources_workspace} -B ${desktop_sources_workspace}/${BUILD_DIR}"
                    sh "cmake --build ${desktop_sources_workspace}/${BUILD_DIR} --target MEGAsync -j1"        
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
