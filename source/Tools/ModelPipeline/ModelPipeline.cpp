// ModelPipeline.cpp : Defines the entry point for the console application.
//
#include "pch.h"
#include "windows.h"
#include "Shlwapi.h"

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif


using namespace ModelPipeline;

int main(int argc, char* argv[])
{
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//_crtBreakAlloc = 29645;
#endif

	std::string inputFile, outputFile;

	if (argc != 3)
	{
		std::runtime_error("Invalid number of arguments!");
	}

	inputFile.assign(argv[1]);
	outputFile.assign(argv[2]);

	if (!inputFile.length())
	{
		std::runtime_error("Invalid Input File Name!");
	}

	if (!outputFile.length())
	{
		std::runtime_error("Invalid Output File Name");
	}

	std::unique_ptr<Model> modelIn(new Model(inputFile, true));


	OutputStreamHelper outStreamHelper(outputFile);
	outStreamHelper << *modelIn;
	outStreamHelper.CloseStream();
	
	InputStreamHelper inputStreamHelper(outputFile);
	std::unique_ptr<Model> modelOut(new Model(outputFile));
	inputStreamHelper.CloseStream();

	if (!modelOut)
	{
		if (modelIn->Meshes().size() != modelOut->Meshes().size())
		{
			throw std::runtime_error("Fatal Error mismatch in Num of Meshes");
		}

		if (modelIn->Materials().size() != modelOut->Materials().size())
		{
			throw std::runtime_error("Fatal Error mismatch in Num of Materials");
		}

		if (modelIn->Bones().size() != modelOut->Bones().size())
		{
			throw std::runtime_error("Fatal Error mismatch in Num of Bones");
		}

		if (modelIn->Animations().size() != modelOut->Animations().size())
		{
			throw std::runtime_error("Fatal Error mismatch in Num of Animations");
		}
	}

	return 0;
}


