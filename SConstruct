# -*- coding: utf-8 -*-
#-------------------------------------------------------------------------#
#   Copyright (C) 2011 by Christoph Thelen                                #
#   doc_bacardi@users.sourceforge.net                                     #
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
#   This program is distributed in the hope that it will be useful,       #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#   GNU General Public License for more details.                          #
#                                                                         #
#   You should have received a copy of the GNU General Public License     #
#   along with this program; if not, write to the                         #
#   Free Software Foundation, Inc.,                                       #
#   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             #
#-------------------------------------------------------------------------#


#----------------------------------------------------------------------------
#
# Set up the Muhkuh Build System.
#
SConscript('mbs/SConscript')
Import('atEnv')

# Create a build environment for the Cortex-R7 and Cortex-A9 based netX chips.
env_cortexR7 = atEnv.DEFAULT.CreateEnvironment(['gcc-arm-none-eabi-4.9', 'asciidoc'])
env_cortexR7.CreateCompilerEnv('NETX4000_RELAXED', ['arch=armv7', 'thumb'], ['arch=armv7-r', 'thumb'])

# Build the platform libraries.
SConscript('platform/SConscript')


#----------------------------------------------------------------------------
#
# Build all files.
#
sources = """
	src/boot_dpm.c
	src/cr7_global_timer.c
	src/memory.c
	src/portcontrol.c
	src/setup_dpm.c
"""

# The list of include folders. Here it is used for all files.
astrIncludePaths = ['src', '#platform/src', '#platform/src/lib']

tEnv = atEnv.NETX4000_RELAXED.Clone()
tEnv.Append(CPPPATH = astrIncludePaths)
tEnv.Append(CPPDEFINES = [['ASIC_ENV_ASIC', '0'], ['ASIC_ENV_SCIT_BOARD', '1'], ['ASIC_ENV_SIMU', '2'], ['ASIC_ENV', '0'], ['CFG_ARTIFICIAL_KEYROM', '0'], ['CFG_ARTIFICIAL_OTP_FUSES', '0']])
tEnv.Replace(LDFILE = 'src/netx4000/netx4000_relaxed_cr7.ld')
tSrc = tEnv.SetBuildPath('targets/netx4000', 'src', sources)
tElf = tEnv.Elf('targets/netx4000/setup_dpm_netx4000_intram.elf', tSrc + tEnv['PLATFORM_LIBRARY'])
tTxt = tEnv.ObjDump('targets/netx4000/setup_dpm_netx4000_intram.txt', tElf, OBJDUMP_FLAGS=['--disassemble', '--source', '--all-headers', '--wide'])
tBin = tEnv.ObjCopy('targets/netx4000/setup_dpm_netx4000_intram.bin', tElf)
tTmp = tEnv.GccSymbolTemplate('targets/netx4000/snippet.xml', tElf, GCCSYMBOLTEMPLATE_TEMPLATE='templates/hboot_snippet.xml', GCCSYMBOLTEMPLATE_BINFILE=tBin[0])

# Create the snippet from the parameters.
global PROJECT_VERSION
aArtifactGroupReverse = ['org', 'muhkuh', 'hboot', 'sniplib']
atSnippet = {
    'group': '.'.join(aArtifactGroupReverse),
    'artifact': 'setup_ddr_netx4000',
    'version': PROJECT_VERSION,
    'vcs_id': tEnv.Version_GetVcsIdLong(),
    'vcs_url': tEnv.Version_GetVcsUrl(),
    'license': 'GPL-2.0',
    'author_name': 'Muhkuh team',
    'author_url': 'https://github.com/muhkuh-sys',
    'description': 'Setup the DPM on a netX4000. The strapping options determine if the parallel DPM, serial DPM or DPM via PCI express should be configured.',
    'categories': ['netx4000', 'booting', 'DDR'],
    'parameter': {
    }
}
strArtifactPath = 'targets/snippets/%s/%s/%s' % ('/'.join(aArtifactGroupReverse), atSnippet['artifact'], PROJECT_VERSION)
snippet_netx90_mpw_com = tEnv.HBootSnippet('%s/%s-%s.xml' % (strArtifactPath, atSnippet['artifact'], PROJECT_VERSION), tTmp, PARAMETER=atSnippet)

# Create the POM file.
tPOM = tEnv.POMTemplate('%s/%s-%s.pom' % (strArtifactPath, atSnippet['artifact'], PROJECT_VERSION), 'templates/pom.xml', POM_TEMPLATE_GROUP=atSnippet['group'], POM_TEMPLATE_ARTIFACT=atSnippet['artifact'], POM_TEMPLATE_VERSION=atSnippet['version'], POM_TEMPLATE_PACKAGING='xml')
