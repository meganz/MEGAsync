def sendSlackNotification(String initialMessage) {
    def resultMessage = """
        ${initialMessage}
        *JOB_URL*: ${env.BUILD_URL}
        *SDK_BRANCH*: \\`${SDK_BRANCH}\\`
    """.stripIndent()
    withCredentials([string(credentialsId: 'slack_webhook_ast_report', variable: 'SLACK_WEBHOOK_AST_REPORT')]) {
        env.RESULT_MESSAGE = resultMessage
        sh """
            curl -X POST -H 'Content-type: application/json' --data '{
                "text": "'"${env.RESULT_MESSAGE}"'"
                }' ${SLACK_WEBHOOK_AST_REPORT}
        """
    }
}
pipeline {
    agent { label 'windows && amd64' }
    options { 
        buildDiscarder(logRotator(numToKeepStr: '20', daysToKeepStr: '15'))
        gitLabConnection('GitLabConnectionJenkins')
    }

    stages {
        stage('Checkout SDK'){
            steps {
                dir('src\\MEGASync\\mega'){
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
            environment {
                VCPKGPATH  = "${desktop_sources_workspace}\\..\\..\\vcpkg"
                MEGAQTPATH = "C:\\Qt\\Qt5.15.13\\5.15.13\\x64"
                _MSPDBSRV_ENDPOINT_ = "${BUILD_TAG}"
                TMP       = "${desktop_sources_workspace}\\tmp"
                TEMP      = "${desktop_sources_workspace}\\tmp"
                TMPDIR    = "${desktop_sources_workspace}\\tmp"
            }
            steps{
                dir(desktop_sources_workspace){
                    sh "mkdir build_dir"
                    sh "mkdir tmp"
                    sh "cmake ${env.BUILD_OPTIONS} -G 'Visual Studio 17 2022' -A x64 -DCMAKE_PREFIX_PATH='${MEGAQTPATH}' -DVCPKG_ROOT='${VCPKGPATH}' -S '${desktop_sources_workspace}' -B '${desktop_sources_workspace}'\\\\build_dir\\\\"
                    sh "cmake --build '${desktop_sources_workspace}'\\\\build_dir\\\\ --config Debug --target MEGAsync -j 1"
                    sh "cmake --build '${desktop_sources_workspace}'\\\\build_dir\\\\ --config Debug --target MEGAupdater -j 1"
                    sh "cmake --build '${desktop_sources_workspace}'\\\\build_dir\\\\ --config Debug --target MEGAShellExt -j 1"            
                }
            }
        }
    }
    post{
        always {
            deleteDir()
        }
        success {
            sendSlackNotification(":white_check_mark: *Jenkins job ${env.JOB_BASE_NAME} succeed*")
        }
        failure {
            sendSlackNotification(":red_circle: *Jenkins job ${env.JOB_BASE_NAME} failed*")
        }
        aborted {
            sendSlackNotification(":red_circle: *Jenkins job ${env.JOB_BASE_NAME} aborted*")
        }
    }
}
