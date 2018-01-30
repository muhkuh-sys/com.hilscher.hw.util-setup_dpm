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
env_cortexR7.CreateCompilerEnv('NETX4000_FULL', ['arch=armv7', 'thumb'], ['arch=armv7-r', 'thumb'])

# Create a build environment for the Cortex-M4 based netX chips.
env_cortexM4 = atEnv.DEFAULT.CreateEnvironment(['gcc-arm-none-eabi-4.9', 'asciidoc'])
env_cortexM4.CreateCompilerEnv('NETX90_MPW', ['arch=armv7', 'thumb'], ['arch=armv7e-m', 'thumb'])

# Build the platform libraries.
SConscript('platform/SConscript')

# Get the project version.
global PROJECT_VERSION


#----------------------------------------------------------------------------
#
# Get the source code version from the VCS.
#
atEnv.DEFAULT.Version('#targets/version/version.h', 'templates/version.h')
atEnv.DEFAULT.Version('#targets/hboot_snippet.xml', 'templates/hboot_snippet.xml')

#----------------------------------------------------------------------------
#
# Build the netx4000 FULL snippet.
#
sources_netx4000_full = """
	src/netx4000_full/boot_dpm.c
	src/netx4000_full/cr7_global_timer.c
	src/netx4000_full/memory.c
	src/netx4000_full/portcontrol.c
	src/netx4000_full/setup_dpm.c
"""

tEnv_netx4000_full = atEnv.NETX4000_FULL.Clone()
tEnv_netx4000_full.Append(CPPPATH = ['src', 'src/netx4000_full', '#platform/src', '#platform/src/lib', '#targets/version'])
tEnv_netx4000_full.Append(CPPDEFINES = [['ASIC_ENV_ASIC', '0'], ['ASIC_ENV_SCIT_BOARD', '1'], ['ASIC_ENV_SIMU', '2'], ['ASIC_ENV', '0'], ['CFG_ARTIFICIAL_KEYROM', '0'], ['CFG_ARTIFICIAL_OTP_FUSES', '0']])
tEnv_netx4000_full.Replace(LDFILE = 'src/netx4000_full/netx4000_full_cr7.ld')
tSrc_netx4000_full = tEnv_netx4000_full.SetBuildPath('targets/netx4000_full', 'src', sources_netx4000_full)
tElf_netx4000_full = tEnv_netx4000_full.Elf('targets/netx4000_full/setup_dpm_netx4000_full_intram.elf', tSrc_netx4000_full + tEnv_netx4000_full['PLATFORM_LIBRARY'])
tTxt_netx4000_full = tEnv_netx4000_full.ObjDump('targets/netx4000_full/setup_dpm_netx4000_full_intram.txt', tElf_netx4000_full, OBJDUMP_FLAGS=['--disassemble', '--source', '--all-headers', '--wide'])
tBin_netx4000_full = tEnv_netx4000_full.ObjCopy('targets/netx4000_full/setup_dpm_netx4000_full_intram.bin', tElf_netx4000_full)
tTmp_netx4000_full = tEnv_netx4000_full.GccSymbolTemplate('targets/netx4000_full/snippet.xml', tElf_netx4000_full, GCCSYMBOLTEMPLATE_TEMPLATE='targets/hboot_snippet.xml', GCCSYMBOLTEMPLATE_BINFILE=tBin_netx4000_full[0])

# Create the snippet from the parameters.
aArtifactGroupReverse = ['com', 'hilscher', 'hw', 'util']
atSnippet_netx4000_full = {
    'group': '.'.join(aArtifactGroupReverse),
    'artifact': 'setup_dpm_netx4000_full',
    'version': PROJECT_VERSION,
    'vcs_id': tEnv_netx4000_full.Version_GetVcsIdLong(),
    'vcs_url': tEnv_netx4000_full.Version_GetVcsUrl(),
    'license': 'GPL-2.0',
    'author_name': 'Hilscher Gesellschaft für Systemautomation',
    'author_url': 'https://github.com/muhkuh-sys',
    'description': 'Setup the DPM on a netX4000 FULL. The strapping options determine if the parallel DPM, serial DPM or DPM via PCI express should be configured.',
    'categories': ['netx4000', 'booting', 'DPM'],
    'parameter': {
    }
}
strArtifactPath_netx4000_full = 'targets/snippets/%s/%s/%s' % ('/'.join(aArtifactGroupReverse), atSnippet_netx4000_full['artifact'], PROJECT_VERSION)
snippet_netx4000_full = tEnv_netx4000_full.HBootSnippet('%s/%s-%s.xml' % (strArtifactPath_netx4000_full, atSnippet_netx4000_full['artifact'], PROJECT_VERSION), tTmp_netx4000_full, PARAMETER=atSnippet_netx4000_full)

# Create the POM file.
tPOM_netx4000_full = tEnv_netx4000_full.POMTemplate('%s/%s-%s.pom' % (strArtifactPath_netx4000_full, atSnippet_netx4000_full['artifact'], PROJECT_VERSION), 'templates/pom.xml', POM_TEMPLATE_GROUP=atSnippet_netx4000_full['group'], POM_TEMPLATE_ARTIFACT=atSnippet_netx4000_full['artifact'], POM_TEMPLATE_VERSION=atSnippet_netx4000_full['version'], POM_TEMPLATE_PACKAGING='xml')


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
tEnv_netx4000_relaxed.Append(CPPPATH = ['src', 'src/netx4000_relaxed', '#platform/src', '#platform/src/lib', '#targets/version'])
tEnv_netx4000_relaxed.Append(CPPDEFINES = [['ASIC_ENV_ASIC', '0'], ['ASIC_ENV_SCIT_BOARD', '1'], ['ASIC_ENV_SIMU', '2'], ['ASIC_ENV', '0'], ['CFG_ARTIFICIAL_KEYROM', '0'], ['CFG_ARTIFICIAL_OTP_FUSES', '0']])
tEnv_netx4000_relaxed.Replace(LDFILE = 'src/netx4000_relaxed/netx4000_relaxed_cr7.ld')
tSrc_netx4000_relaxed = tEnv_netx4000_relaxed.SetBuildPath('targets/netx4000_relaxed', 'src', sources_netx4000_relaxed)
tElf_netx4000_relaxed = tEnv_netx4000_relaxed.Elf('targets/netx4000_relaxed/setup_dpm_netx4000_relaxed_intram.elf', tSrc_netx4000_relaxed + tEnv_netx4000_relaxed['PLATFORM_LIBRARY'])
tTxt_netx4000_relaxed = tEnv_netx4000_relaxed.ObjDump('targets/netx4000_relaxed/setup_dpm_netx4000_relaxed_intram.txt', tElf_netx4000_relaxed, OBJDUMP_FLAGS=['--disassemble', '--source', '--all-headers', '--wide'])
tBin_netx4000_relaxed = tEnv_netx4000_relaxed.ObjCopy('targets/netx4000_relaxed/setup_dpm_netx4000_relaxed_intram.bin', tElf_netx4000_relaxed)
tTmp_netx4000_relaxed = tEnv_netx4000_relaxed.GccSymbolTemplate('targets/netx4000_relaxed/snippet.xml', tElf_netx4000_relaxed, GCCSYMBOLTEMPLATE_TEMPLATE='targets/hboot_snippet.xml', GCCSYMBOLTEMPLATE_BINFILE=tBin_netx4000_relaxed[0])

# Create the snippet from the parameters.
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
sources_netx90_mpw = """
	src/netx90_mpw/setup_dpm.c
	src/header.c
"""

tEnv_netx90_mpw = atEnv.NETX90_MPW.Clone()
tEnv_netx90_mpw.Append(CPPPATH = ['src', 'src/netx90_mpw', '#platform/src', '#platform/src/lib', '#targets/version'])
tEnv_netx90_mpw.Append(CPPDEFINES = [['ASIC_ENV_ASIC', '0'], ['ASIC_ENV_SCIT_BOARD', '1'], ['ASIC_ENV_SIMU', '2'], ['ASIC_ENV', '0'], ['CFG_ARTIFICIAL_KEYROM', '0'], ['CFG_ARTIFICIAL_OTP_FUSES', '0']])
tEnv_netx90_mpw.Replace(LDFILE = 'src/netx90_mpw/netx90_mpw.ld')
tSrc_netx90_mpw = tEnv_netx90_mpw.SetBuildPath('targets/netx90_mpw', 'src', sources_netx90_mpw)
tElf_netx90_mpw = tEnv_netx90_mpw.Elf('targets/netx90_mpw/setup_dpm_netx90_mpw_intram.elf', tSrc_netx90_mpw + tEnv_netx90_mpw['PLATFORM_LIBRARY'])
tTxt_netx90_mpw = tEnv_netx90_mpw.ObjDump('targets/netx90_mpw/setup_dpm_netx90_mpw_intram.txt', tElf_netx90_mpw, OBJDUMP_FLAGS=['--disassemble', '--source', '--all-headers', '--wide'])
tBin_netx90_mpw = tEnv_netx90_mpw.ObjCopy('targets/netx90_mpw/setup_dpm_netx90_mpw_intram.bin', tElf_netx90_mpw)
tTmp_netx90_mpw = tEnv_netx90_mpw.GccSymbolTemplate('targets/netx90_mpw/snippet.xml', tElf_netx90_mpw, GCCSYMBOLTEMPLATE_TEMPLATE='targets/hboot_snippet.xml', GCCSYMBOLTEMPLATE_BINFILE=tBin_netx90_mpw[0])

# Create the snippet from the parameters.
aArtifactGroupReverse = ['com', 'hilscher', 'hw', 'util']
atSnippet_netx90_mpw = {
    'group': '.'.join(aArtifactGroupReverse),
    'artifact': 'setup_dpm_netx90_mpw',
    'version': PROJECT_VERSION,
    'vcs_id': tEnv_netx90_mpw.Version_GetVcsIdLong(),
    'vcs_url': tEnv_netx90_mpw.Version_GetVcsUrl(),
    'license': 'GPL-2.0',
    'author_name': 'Hilscher Gesellschaft für Systemautomation',
    'author_url': 'https://github.com/muhkuh-sys',
    'description': 'Setup the DPM on a netX90 MPW. The strapping options determine if the parallel or serial DPM should be configured.',
    'categories': ['netx90', 'booting', 'DPM'],
    'parameter': {
        'CONFIGURATION_ADDRESS': {'help': 'The address of the hardware config.', 'default': 0}
    }
}
strArtifactPath_netx90_mpw = 'targets/snippets/%s/%s/%s' % ('/'.join(aArtifactGroupReverse), atSnippet_netx90_mpw['artifact'], PROJECT_VERSION)
snippet_netx90_mpw = tEnv_netx90_mpw.HBootSnippet('%s/%s-%s.xml' % (strArtifactPath_netx90_mpw, atSnippet_netx90_mpw['artifact'], PROJECT_VERSION), tTmp_netx90_mpw, PARAMETER=atSnippet_netx90_mpw)

# Create the POM file.
tPOM_netx90_mpw = tEnv_netx90_mpw.POMTemplate('%s/%s-%s.pom' % (strArtifactPath_netx90_mpw, atSnippet_netx90_mpw['artifact'], PROJECT_VERSION), 'templates/pom.xml', POM_TEMPLATE_GROUP=atSnippet_netx90_mpw['group'], POM_TEMPLATE_ARTIFACT=atSnippet_netx90_mpw['artifact'], POM_TEMPLATE_VERSION=atSnippet_netx90_mpw['version'], POM_TEMPLATE_PACKAGING='xml')
