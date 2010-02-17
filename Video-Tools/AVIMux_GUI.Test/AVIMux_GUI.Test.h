// AVIMux_GUI.Test.h

#pragma once

#define _WIN32_WINNT 0x0502
#include "windows.h"

/*
#define COMMON_TEST_FILE_PATH "..\\..\\..\\Common\\Common.Tests\\Files\\TextFiles"
#include "../../Common/Common.Tests/Path.test.h"
#include "../../Common/Common.Tests/UTF-8.test.h"
#include "../../Common/Common.Tests/TextFile.test.h"
*/
#define AAC_TEST_FILE_PATH "..\\..\\AudioClassTests\\Files\\AAC"
//#include "AudioSource_AAC.Test.h" These need special test files, TODO: include test data as for AC3/DTS
#include "AudioSource_DTS.Test.h"
#include "AudioSource_AC3.Test.h"

using namespace System;

namespace AMG 
{

}
