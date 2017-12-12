docker.image('jenkins-ubuntu-1604').inside {
	stage 'Clean before build'
	sh 'rm -rf .[^.] .??* *'

	stage 'Checkout'
	checkout([$class: 'GitSCM', branches: [[name: '*/master']], doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'SubmoduleOption', disableSubmodules: false, recursiveSubmodules: true, reference: '', trackingSubmodules: false]], submoduleCfg: [], userRemoteConfigs: [[url: 'https://github.com/muhkuh-sys/com.hilscher.hw.util-setup_dpm.git']]])

	stage 'Build'
	sh 'python2.7 mbs/mbs'

	stage 'Save Artifacts'
	archive 'targets/snippets/**/*.xml,targets/snippets/**/*.pom'

	stage 'Clean after build'
	sh 'rm -rf .[^.] .??* *'
}
