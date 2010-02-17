#pragma once

#include "../Cache.h"
#include "../BitStreamFactory.h"
#include "../FileStream.h"
#include "../AVIMux_GUI/AudioSource_AAC.h"

using namespace System;
using namespace System::IO;
using namespace System::Reflection;
using namespace NUnit::Framework;

namespace AMG { namespace AAC 
{
	public ref class Test_ADTS_Reader_base abstract
	{
	protected:
		String^ AdtsFile_NoCRC_1BPF;

	public:
		[TestFixtureSetUp]
		void Test_SetupFixture()
		{
			Assembly^ assembly = Assembly::GetExecutingAssembly();
			Uri^ assemblyUri = gcnew Uri(assembly->CodeBase);
			FileInfo^ fi = gcnew FileInfo(assemblyUri->LocalPath);
			String^ executingDir = fi->Directory->FullName;

			AdtsFile_NoCRC_1BPF = Path::GetFullPath(Path::Combine(executingDir, Path::Combine(AAC_TEST_FILE_PATH, "ap02_48.adts")));
		}

		[SetUp]
		void Setup()
		{
		}

	protected:
		AACSOURCE::AdtsFrameHeader GetFirstPackageHeaderFromFile(String^ name)
		{
			CFileStream* fs = new CFileStream();
			pin_ptr<const wchar_t> ptrFilename = PtrToStringChars(name);
			int openResult = fs->Open(ptrFilename, StreamMode::Read);
			Assert::AreEqual(openResult, STREAM_OK);

			IBitStream* bts = CBitStreamFactory::CreateInstance<CBitStream2>();
			bts->Open(fs);

			AACSOURCE::AdtsFrameHeader hdr;
			hdr.Read(*bts);

			bts->Close();
			delete bts;
			fs->Close();
			delete fs;

			return hdr;
		}

	public:
		property String^ TestFile
		{
			virtual String^ get() abstract;
		}

	public:
		[Test]
		void Test_Open()
		{
			AACSOURCE::AdtsFrameHeader header = GetFirstPackageHeaderFromFile(TestFile);
		}
	};

	[TestFixture]
	public ref class Test_read_adts_header_ap02 : Test_ADTS_Reader_base
	{
	public:
		property String^ TestFile {
			virtual String^ get() override { return AdtsFile_NoCRC_1BPF; }
		}
	public:
		[Test]
		void Test_Read_MPEGID()
		{
			AACSOURCE::AdtsFrameHeader header = GetFirstPackageHeaderFromFile(TestFile);
			Assert::AreEqual(header.ID, AACSOURCE::MPEGID::MPEG4);
		}

		[Test]
		void Test_Read_ProtectionAbsent()
		{
			AACSOURCE::AdtsFrameHeader header = GetFirstPackageHeaderFromFile(TestFile);
			Assert::AreEqual(header.protection_absent, 1);
		}

		[Test]
		void Test_Read_BlocksPerFrame()
		{
			AACSOURCE::AdtsFrameHeader header = GetFirstPackageHeaderFromFile(TestFile);
			Assert::AreEqual(header.number_of_data_blocks, 1);
		}

		[Test]
		void Test_Read_SampleFrequency()
		{
			AACSOURCE::AdtsFrameHeader header = GetFirstPackageHeaderFromFile(TestFile);
			Assert::AreEqual(header.sampling_fequency_index, 3);
		}

		[Test]
		void Test_Read_Profile()
		{
			AACSOURCE::AdtsFrameHeader header = GetFirstPackageHeaderFromFile(TestFile);
			Assert::AreEqual(header.profile, AACSOURCE::AdtsProfile::LTP);
		}

		[Test]
		void Test_Read_Layer()
		{
			AACSOURCE::AdtsFrameHeader header = GetFirstPackageHeaderFromFile(TestFile);
			Assert::AreEqual(header.layer, 0);
		}

		[Test]
		void Test_Read_SyncWord()
		{
			AACSOURCE::AdtsFrameHeader header = GetFirstPackageHeaderFromFile(TestFile);
			Assert::AreEqual(header.syncword, 0xFFF);
		}

		[Test]
		void Test_Read_ChannelConfiguration()
		{
			AACSOURCE::AdtsFrameHeader header = GetFirstPackageHeaderFromFile(TestFile);
			Assert::AreEqual(header.channel_configuration, 2);
		}
	};
}}
