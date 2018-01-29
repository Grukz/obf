/*

Copyright (c) 2018, ITHare.com
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

//HELPER file for testing, generates randomized command lines for a testing session
//  Using .cpp to avoid writing the same logic twice in *nix .sh and Win* .bat

#include <iostream>
#include <string.h>
#include <string>
#include <stdio.h>
#include <assert.h>

std::string srcDirPrefix = "";

#if defined(__APPLE_CC__) || defined(__linux__)
std::string rootTestFolder() { return  srcDirPrefix + "../"; }

std::string buildRelease(std::string defines) {
	return std::string("g++ -O3 -DNDEBUG ") + defines + " -o obftemp -std=c++1z -lstdc++ -Werror " + srcDirPrefix + "../official.cpp";
}
std::string buildDebug(std::string defines) {
	return std::string("g++ ") + defines + " -o obftemp -std=c++1z -lstdc++ -Werror " + srcDirPrefix + "../official.cpp";
}
std::string build32option() {
	return " -m32";
}
std::string genRandom64() {
	static FILE* frnd = fopen("/dev/urandom","rb");
	if(frnd==0) {
		std::cout << "Cannot open /dev/urandom, aborting" << std::endl;
		abort();
	}
	uint8_t buf[8];
	size_t rd = fread(buf,sizeof(buf),1,frnd);
	if( rd != 1 ) {
		std::cout << "Problems reading from /dev/urandom, aborting" << std::endl;
		abort();
	}

	char buf2[sizeof(buf)*2+1];
	for(size_t i=0; i < sizeof(buf) ; ++i) {
		assert(buf[i]<=255);
		sprintf(&buf2[i*2],"%02x",buf[i]);
	}
	return std::string(buf2);
}
std::string exitCheck(std::string cmd, bool expectok = true) {
	if( expectok )
		return std::string("if [ ! $? -eq 0 ]; then\n  echo \"") + cmd + ( "\">failed.sh\n  exit 1\nfi");
	else
		return std::string("if [ ! $? -ne 0 ]; then\n  echo \"") + cmd + ( "\">failed.sh\n  exit 1\nfi");
}
std::string echo(std::string s,bool highlight=false) {
	if(highlight)
		return std::string("echo \"${HIGHLIGHT}")+s+"${NOHIGHLIGHT}\"";	
	else
		return std::string("echo \"" + s +"\"");
}
std::string run(std::string redirect) {
	if(redirect!="")
		return std::string("./obftemp >")+redirect;
	else
		return std::string("./obftemp");
}
std::string checkObfuscation(bool obfuscated) {//very weak heuristics, but still better than nothing
	std::string ret = std::string("strings obftemp | grep Negative");//referring to string "Negative value of factorial()" 
	return ret + "\n" + exitCheck(ret,!obfuscated);
}
std::string backupExe() {
	return std::string("mv -f obftemp obftemp-release");
}
std::string setup() {
	return std::string("#!/bin/sh\nHIGHLIGHT='\033[32m\033[1m\033[7m'\nNOHIGHLIGHT='\033[0m'\n") + echo("===*** COMPILER BEING USED: ***===",true)+"\ng++ --version";
		//color along the lines of https://stackoverflow.com/a/5947802/4947867
}
std::string cleanup() {
	return std::string("rm obftemp");	
}
#elif defined(_WIN32)
#include <windows.h>
std::string rootTestFolder() { return  srcDirPrefix + "..\\"; }

std::string replace_string(std::string subject, std::string search,//adapted from https://stackoverflow.com/a/14678964
	std::string replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}
std::string buildRelease(std::string defines_) {
	std::string defines = replace_string(defines_, " -D", " /D");
	return std::string("cl /permissive- /GS /GL /W3 /Gy /Zc:wchar_t /Gm- /O2 /sdl /Zc:inline /fp:precise /DNDEBUG /D_CONSOLE /D_UNICODE /DUNICODE /errorReport:prompt /WX /Zc:forScope /GR- /Gd /Oi /MT /EHsc /nologo /diagnostics:classic /std:c++17 /cgthreads1 /INCREMENTAL:NO") + defines + " " + srcDirPrefix + "..\\official.cpp";
		//string is copy-pasted from Rel-NoPDB config with manually-added /cgthreads1 /INCREMENTAL:NO, and /WX- replaced with /WX
}
std::string buildDebug(std::string defines_) {
	std::string defines = replace_string(defines_, " -D", " /D");
	return std::string("cl /permissive- /GS /W3 /Zc:wchar_t /ZI /Gm /Od /sdl /Zc:inline /fp:precise /D_DEBUG /D_CONSOLE /D_UNICODE /DUNICODE /errorReport:prompt /WX /Zc:forScope /RTC1 /Gd /MDd /EHsc /nologo /diagnostics:classic /std:c++17 /cgthreads1 /INCREMENTAL:NO /bigobj") + defines + " " + srcDirPrefix + "..\\official.cpp";
		//string is copy-pasted from Debug config with manually-added /cgthreads1 /INCREMENTAL:NO /bigobj, and /WX- replaced with /WX
}
std::string build32option() {
	std::cout << "no option to run both 32-bit and 64-bit testing for MSVC now, run testing without -add32tests in two different 'Tools command prompts' instead" << std::endl;
	abort();
}

std::string genRandom64() {
	static HCRYPTPROV prov = NULL;
	if (!prov) {
		BOOL ok = CryptAcquireContext(&prov,NULL,NULL,PROV_RSA_FULL,0);
		if (!ok) {
			std::cout << "CryptAcquireContext() returned error, aborting" << std::endl;
			abort();
		}
	};
	uint8_t buf[8];
	BOOL ok = CryptGenRandom(prov, sizeof(buf), buf);
	if (!ok) {
		std::cout << "Problems calling CryptGenRandom(), aborting" << std::endl;
		abort();
	}

	char buf2[sizeof(buf) * 2 + 1];
	for (size_t i = 0; i < sizeof(buf); ++i) {
		assert(buf[i] <= 255);
		sprintf(&buf2[i * 2], "%02x", buf[i]);
	}
	return std::string(buf2);
}
std::string exitCheck(std::string cmd,bool expectok = true) {
	static int nextlabel = 1;
	if (expectok) {

		auto ret = std::string("IF NOT ERRORLEVEL 1 GOTO LABEL") + std::to_string(nextlabel)
		    + "\nECHO " + replace_string(cmd,">","^>") + ">failed.bat"
			+ "\nEXIT /B\n:LABEL" + std::to_string(nextlabel);
		nextlabel++;
		return ret;
	}
	else {
		auto ret = std::string("IF ERRORLEVEL 1 GOTO LABEL") + std::to_string(nextlabel)
		    + "\nECHO " + replace_string(cmd, ">", "^>") + ">failed.bat"
			+ "\nEXIT /B\n:LABEL" + std::to_string(nextlabel);
		nextlabel++;
		return ret;
	}
}
std::string echo(std::string s,bool highlight=false) {
	return std::string("ECHO " + replace_string(s, ">", "^>") +"");
}
std::string run(std::string redirect) {
	if(redirect!="")
		return std::string("official.exe >")+redirect;
	else
		return std::string("official.exe");
}
std::string backupExe() {
	return std::string("MOVE /Y official.exe official-release.exe");
}
std::string checkObfuscation(bool obfuscated) {//very weak heuristics, but still better than nothing
	return "";//sorry, nothing for now for Windows :-(
}
std::string cleanup() {
	return std::string("del official.exe");
}
std::string setup() {
	return "@ECHO OFF\nDEL *.PDB\nDEL *.IDB";
}
#else
#error "Unrecognized platform for randomized testing"
#endif 

bool add32tests = false;

std::string fixedSeeds() {
	return std::string(" -DITHARE_OBF_SEED=0x4b295ebab3333abc -DITHARE_OBF_SEED2=0x36e007a38ae8e0ea");//from random.org
}

std::string genSeed() {
	return std::string(" -DITHARE_OBF_SEED=0x") + genRandom64();
}

std::string genSeeds() {
	return genSeed() + " -DITHARE_OBF_SEED2=0x" + genRandom64();
}

int usage() {
	std::cout << "Usage:" << std::endl;
	std::cout << "helper [-nodefinetests] <nrandomtests>" << std::endl; 
	return 1;
}

void issueCommand(std::string cmd) {
	std::cout << echo(cmd) << std::endl;
	std::cout << cmd << std::endl;
}

enum class write_output { none, stable, random };

void buildCheckRunCheck(std::string cmd,bool obfuscated,write_output wo) {
	issueCommand(cmd);
	std::cout << exitCheck(cmd) << std::endl;
	std::cout << checkObfuscation(obfuscated) << std::endl;

	std::string tofile = "";
	switch (wo) {
	case write_output::none:
		break;
	case write_output::stable:
		tofile = rootTestFolder() + "obftemp.txt";
		break;
	case write_output::random:
		tofile = "local_obftemp.txt";
		break;
	}
	std::string cmdrun = run(tofile);
	issueCommand(cmdrun);
	std::cout << exitCheck(cmdrun) << std::endl;
}

std::string seedsByNum(int nseeds) {
	assert(nseeds >= -1 && nseeds <= 2);
	if(nseeds==1)
		return genSeed();
	else if(nseeds==2)
		return genSeeds();
	else if(nseeds==-1)
		return fixedSeeds();
	assert(nseeds==0);
	return "";
}

enum class config { debug, release };

std::string buildCmd(config cfg,std::string defs) {
	switch(cfg) {
	case config::debug:
		return buildDebug(defs);
	case config::release:
		return buildRelease(defs);
	}
	assert(false);
	return "";
}

void buildCheckRunCheckx2(config cfg,std::string defs,int nseeds, bool obfuscated=true,write_output wo=write_output::none) {
	assert(nseeds >= -1 && nseeds <= 2);
	if(wo==write_output::stable){
		assert(nseeds<0);
	}
	else {
		assert(nseeds>=0);
	}
	
	std::string cmd1 = buildCmd(cfg, defs + seedsByNum(nseeds));
	buildCheckRunCheck(cmd1,obfuscated,wo);
	std::string cmd2 = buildCmd(cfg, defs + seedsByNum(nseeds) + " -DITHARE_OBF_TEST_NO_NAMESPACE");
	buildCheckRunCheck(cmd2,obfuscated,wo);
	
	if(add32tests) {
		std::string m32 = build32option();
		std::string cmd1 = buildCmd(cfg, defs + m32 + seedsByNum(nseeds));
		buildCheckRunCheck(cmd1,obfuscated,wo);
		std::string cmd2 = buildCmd(cfg, defs + m32 + seedsByNum(nseeds) + " -DITHARE_OBF_TEST_NO_NAMESPACE");
		buildCheckRunCheck(cmd2,obfuscated,wo);
	}
}

void genDefineTests() {
	std::cout << echo(std::string("=== -Define Test 1/14 (DEBUG, -DITHARE_OBF_ENABLE_AUTO_DBGPRINT, write_output::stable) ==="),true) << std::endl;
	buildCheckRunCheckx2(config::debug, " -DITHARE_OBF_INIT -DITHARE_OBF_CONSISTENT_XPLATFORM_IMPLICIT_SEEDS -DITHARE_OBF_ENABLE_AUTO_DBGPRINT", -1, true, write_output::stable);
	std::cout << echo(std::string("=== -Define Test 2/14 (RELEASE, -DITHARE_OBF_ENABLE_AUTO_DBGPRINT=2, write_output::random)==="),true) << std::endl;
	buildCheckRunCheckx2(config::release, " -DITHARE_OBF_INIT -DITHARE_OBF_CONSISTENT_XPLATFORM_IMPLICIT_SEEDS -DITHARE_OBF_ENABLE_AUTO_DBGPRINT=2", 2, true, write_output::random);
	std::cout << echo( std::string("=== -Define Test 3/14 (DEBUG, no ITHARE_OBF_SEED) ===",true) ) << std::endl;
	buildCheckRunCheckx2(config::debug,"",0,false);
	std::cout << echo( std::string("=== -Define Test 4/14 (RELEASE, no ITHARE_OBF_SEED) ===" ),true) << std::endl;
	buildCheckRunCheckx2(config::release,"",0,false);
	std::cout << echo( std::string("=== -Define Test 5/14 (DEBUG, single ITHARE_OBF_SEED) ===" ),true) << std::endl;
	buildCheckRunCheckx2(config::debug,"",1);
	std::cout << echo( std::string("=== -Define Test 6/14 (RELEASE, single ITHARE_OBF_SEED) ===" ),true) << std::endl;
	buildCheckRunCheckx2(config::release,"",1);
	std::cout << echo( std::string("=== -Define Test 7/14 (DEBUG) ===",true) ) << std::endl;
	buildCheckRunCheckx2(config::debug,"",2);
	std::cout << echo( std::string("=== -Define Test 8/14 (RELEASE) ===",true ) ) << std::endl;
	buildCheckRunCheckx2(config::release,"",2);
	std::cout << echo(std::string("=== -Define Test 9/14 (DEBUG, -DITHARE_OBF_NO_ANTI_DEBUG) ===")) << std::endl;
	buildCheckRunCheckx2(config::debug, " -DITHARE_OBF_NO_ANTI_DEBUG", 2);
	std::cout << echo(std::string("=== -Define Test 10/14 (RELEASE, -DITHARE_OBF_NO_IMPLICIT_ANTI_DEBUG) ===")) << std::endl;
	buildCheckRunCheckx2(config::release, " -DITHARE_OBF_NO_IMPLICIT_ANTI_DEBUG", 2);
	std::cout << echo( std::string("=== -Define Test 11/14 (DEBUG, -DITHARE_OBF_DBG_RUNTIME_CHECKS) ===" ) ) << std::endl;
#if defined(_MSC_VER)
	std::cout << echo("*** SKIPPED -DITHARE_OBF_DBG_RUNTIME_CHECKS FOR MSVC (cannot cope) ***") << std::endl;
#else
	buildCheckRunCheckx2(config::debug, " -DITHARE_OBF_DBG_RUNTIME_CHECKS", 2);
#endif
	std::cout << echo( std::string("=== -Define Test 12/14 (RELEASE, -DITHARE_OBF_DBG_RUNTIME_CHECKS) ===" ) ) << std::endl;
#if defined(_MSC_VER)
	std::cout << echo("*** SKIPPED -DITHARE_OBF_DBG_RUNTIME_CHECKS FOR MSVC (cannot cope) ***") << std::endl;
#else
	buildCheckRunCheckx2(config::release, " -DITHARE_OBF_DBG_RUNTIME_CHECKS",2);
#endif
	std::cout << echo(std::string("=== -Define Test 13/14 (DEBUG, -DITHARE_OBF_CRYPTO_PRNG) ===")) << std::endl;
#if defined(_MSC_VER) && !defined(_M_X64)
	std::cout << echo("*** SKIPPED -DITHARE_OBF_DBG_CRYPTO_PRNG FOR MSVC/x86 (cannot cope) ***") << std::endl;
#elif defined(__GNUC__)
	std::cout << echo("*** SKIPPED -DITHARE_OBF_DBG_CRYPTO_PRNG FOR GCC (bug?) ***") << std::endl;
#else
	buildCheckRunCheckx2(config::debug," -DITHARE_OBF_CRYPTO_PRNG",2);
#endif
	std::cout << echo(std::string("=== -Define Test 14/14 (RELEASE, -DITHARE_OBF_CRYPTO_PRNG) ===")) << std::endl;
#if defined(_MSC_VER) && !defined(_M_X64)
	std::cout << echo("*** SKIPPED -DITHARE_OBF_DBG_CRYPTO_PRNG FOR MSVC/x86 (cannot cope) ***") << std::endl;
#elif defined(__GNUC__)
	std::cout << echo("*** SKIPPED -DITHARE_OBF_DBG_CRYPTO_PRNG FOR GCC (bug?) ***") << std::endl;
#else
	buildCheckRunCheckx2(config::release, " -DITHARE_OBF_CRYPTO_PRNG", 2);
#endif
}

void genRandomTests(size_t n) {
	for (size_t i = 0; i < n; ++i) {
		config cfg = config::release;
		if (i % 4 == 0)
			cfg = config::debug;
		std::string extra;
		if (i % 3 == 0) { //every third, non-exclusive
			bool rtchecks_ok = true;
#if defined(_MSC_VER) 
			rtchecks_ok = false;//cl doesn't seem to cope well with RUNTIME_CHECKS :-(
#endif
			if(rtchecks_ok)
				extra += " -DITHARE_OBF_DBG_RUNTIME_CHECKS";
		}
		if(add32tests && i%5 <=1)
			extra += build32option();
		std::cout << echo( std::string("=== Random Test ") + std::to_string(i+1) + "/" + std::to_string(n) + " ===" ) << std::endl;
		std::string defines = genSeeds()+" -DITHARE_OBF_INIT -DITHARE_OBF_CONSISTENT_XPLATFORM_IMPLICIT_SEEDS"+extra;
		//std::string defines = genSeeds() + " -DITHARE_OBF_NO_ANTI_DEBUG -DITHARE_OBF_CONSISTENT_XPLATFORM_IMPLICIT_SEEDS" + extra;
		if( cfg == config::debug ) 
			buildCheckRunCheck(buildDebug(defines),true,write_output::none);
		else {
			assert(cfg == config::release);
			buildCheckRunCheck(buildRelease(defines), true, write_output::none);
			if (extra == "") {
				std::cout << echo("Backing up release executable...") << std::endl;
				std::cout << backupExe() << std::endl;
			}
		}
	}
}

void genSetup() {
	std::cout << setup() << std::endl;
}

void genCleanup() {
	std::cout << cleanup() << std::endl;
}

int main(int argc, char** argv) {
	bool nodefinetests = false;
	
	int argcc = 1;
	while(argcc<argc) {
		if(strcmp(argv[argcc],"-nodefinetests") == 0) {
			nodefinetests = true;
			argcc++;
		}
		else if(strcmp(argv[argcc],"-add32tests") == 0) {
			add32tests = true;
			argcc++;
		}
		else if (strcmp(argv[argcc], "-srcdirprefix") == 0) {
			assert(argcc +1 < argc);
			srcDirPrefix = argv[argcc+1];
			argcc+=2;
		}
		//other options go here
		else
			break;
	}
	
	if(argcc >= argc)
		return usage();
	char* end;
	unsigned long nrandom = strtoul(argv[argcc],&end,10);
	if(*end!=0)
		return usage();
		
	genSetup();
	if(!nodefinetests)
		genDefineTests();

	genRandomTests(nrandom);
	genCleanup();
}
