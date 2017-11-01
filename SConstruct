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
# Build the netx4000 RELAXED snippet.
#
sources_netx4000_relaxed = """
	src/netx4000_relaxed/boot_dpm.c
	src/netx4000_relaxed/cr7_global_timer.c
	src/netx4000_relaxed/memory.c
	src/netx4000_relaxed/portcontrol.c
	src/netx4000_relaxed/setup_dpm.c
"""

tEnv_netx4000_relaxed = atEnv.NETX4000_RELAXED.Clone()
tEnv_netx4000_relaxed.Append(CPPPATH = ['src', 'src/netx4000_relaxed', '#platform/src', '#platform/src/lib'])
tEnv_netx4000_relaxed.Append(CPPDEFINES = [['ASIC_ENV_ASIC', '0'], ['ASIC_ENV_SCIT_BOARD', '1'], ['ASIC_ENV_SIMU', '2'], ['ASIC_ENV', '0'], ['CFG_ARTIFICIAL_KEYROM', '0'], ['CFG_ARTIFICIAL_OTP_FUSES', '0']])
tEnv_netx4000_relaxed.Replace(LDFILE = 'src/netx4000_relaxed/netx4000_relaxed_cr7.ld')
tSrc_netx4000_relaxed = tEnv_netx4000_relaxed.SetBuildPath('targets/netx4000_relaxed', 'src', sources_netx4000_relaxed)
tElf_netx4000_relaxed = tEnv_netx4000_relaxed.Elf('targets/netx4000_relaxed/setup_dpm_netx4000_relaxed_intram.elf', tSrc_netx4000_relaxed + tEnv_netx4000_relaxed['PLATFORM_LIBRARY'])
tTxt_netx4000_relaxed = tEnv_netx4000_relaxed.ObjDump('targets/netx4000_relaxed/setup_dpm_netx4000_relaxed_intram.txt', tElf_netx4000_relaxed, OBJDUMP_FLAGS=['--disassemble', '--source', '--all-headers', '--wide'])
tBin_netx4000_relaxed = tEnv_netx4000_relaxed.ObjCopy('targets/netx4000_relaxed/setup_dpm_netx4000_relaxed_intram.bin', tElf_netx4000_relaxed)
tTmp_netx4000_relaxed = tEnv_netx4000_relaxed.GccSymbolTemplate('targets/netx4000_relaxed/snippet.xml', tElf_netx4000_relaxed, GCCSYMBOLTEMPLATE_TEMPLATE='templates/hboot_snippet.xml', GCCSYMBOLTEMPLATE_BINFILE=tBin_netx4000_relaxed[0])

# Create the snippet from the parameters.
global PROJECT_VERSION
aArtifactGroupReverse = ['com', 'hilscher', 'hw', 'util']
atSnippet_netx4000_relaxed = {
    'group': '.'.join(aArtifactGroupReverse),
    'artifact': 'setup_dpm_netx4000_relaxed',
    'version': PROJECT_VERSION,
    'vcs_id': tEnv_netx4000_relaxed.Version_GetVcsIdLong(),
    'vcs_url': tEnv_netx4000_relaxed.Version_GetVcsUrl(),
    'license': 'GPL-2.0',
    'author_name': 'Hilscher Gesellschaft für Systemautomation',
    'author_url': 'https://github.com/muhkuh-sys',
    'description': 'Setup the DPM on a netX4000 RELAXED. The strapping options determine if the parallel DPM, serial DPM or DPM via PCI express should be configured.',
    'categories': ['netx4000', 'booting', 'DPM'],
    'parameter': {
    }
}
strArtifactPath_netx4000_relaxed = 'targets/snippets/%s/%s/%s' % ('/'.join(aArtifactGroupReverse), atSnippet_netx4000_relaxed['artifact'], PROJECT_VERSION)
snippet_netx4000_relaxed = tEnv_netx4000_relaxed.HBootSnippet('%s/%s-%s.xml' % (strArtifactPath_netx4000_relaxed, atSnippet_netx4000_relaxed['artifact'], PROJECT_VERSION), tTmp_netx4000_relaxed, PARAMETER=atSnippet_netx4000_relaxed)

# Create the POM file.
tPOM_netx4000_relaxed = tEnv_netx4000_relaxed.POMTemplate('%s/%s-%s.pom' % (strArtifactPath_netx4000_relaxed, atSnippet_netx4000_relaxed['artifact'], PROJECT_VERSION), 'templates/pom.xml', POM_TEMPLATE_GROUP=atSnippet_netx4000_relaxed['group'], POM_TEMPLATE_ARTIFACT=atSnippet_netx4000_relaxed['artifact'], POM_TEMPLATE_VERSION=atSnippet_netx4000_relaxed['version'], POM_TEMPLATE_PACKAGING='xml')


#----------------------------------------------------------------------------
#
# Build the netx90 MPW snippet.
#
