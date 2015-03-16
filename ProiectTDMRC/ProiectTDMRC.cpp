#include "DetectLeaks.h"
#include "stdafx.h"

#include "Direct_Access_Image.h"
#include "GenerateImage.h"
#include "ElementalDust.h"

#include "StudentCodec.h"

size_t getFileContents(char *file_name, void **file_contents)
{
	FILE *fin;
	fopen_s(&fin, file_name, "rb");

	size_t file_size_uint = 0;

	fseek(fin, 0L, SEEK_END);
	file_size_uint = ftell(fin);
	fseek(fin, 0L, SEEK_SET);

	*file_contents = new unsigned char[file_size_uint];

	fread(*file_contents, sizeof(unsigned char), file_size_uint, fin);

	fclose (fin);

	return file_size_uint;
}

void writeFileContents(char *file_name, void *file_contents, size_t file_size_uint)
{
	FILE *fout;
	fopen_s(&fout, file_name, "wb");

	fwrite(file_contents, 1, file_size_uint, fout);

	fclose (fout);
}

const char* Compare(void *original_data_p, size_t original_size, void *decompressed_p, size_t decompressed_size)
{
	if(original_size != decompressed_size)
		return "The buffer sizes differ.\n";

	unsigned char *original_data = (unsigned char *)original_data_p;
	unsigned char *decompressed = (unsigned char *)decompressed_p;

	for(size_t i=0; i<original_size; i++)
	{
		if(original_data[i] != decompressed[i])
			return "The buffer contents differ.\n";
	}

	return "The buffers are identical.\n";
}

void test()
{
	//set image properties
	float km=4;
	int dim = 31;

	//generate a image
	KImage *perlin_noise_img = new KImage(dim, dim, 8);
	GenerateImage::perlinNoise(perlin_noise_img);
	perlin_noise_img->SaveAs("perlin_noise.tif");
	delete perlin_noise_img;

	//open a image and convert it to edp
	KImage *dustmap_img = new KImage("perlin_noise.tif");
	ElementalDust ed = ElementalDust();
	ed.import(dustmap_img, km);
	ed.SaveAs("perlin_noise.edp");
	ed = ElementalDust();
	delete dustmap_img;

	//open an edp and compress to edpc
	void *original_data;
	size_t original_size;
	void *compressed;
	size_t compressed_size;
	original_size = getFileContents("perlin_noise.edp", &original_data);
	compressed_size = StudentCodec::compress(original_data, original_size, &compressed);
	writeFileContents("perlin_noise.edpc", compressed, compressed_size);
	delete[] compressed;
	delete[] original_data;

	//Open an edpc and decompress to edp
	void *decompressed;
	size_t decompressed_size;
	compressed_size = getFileContents("perlin_noise.edpc", &compressed);
	decompressed_size = StudentCodec::decompress(compressed, compressed_size, &decompressed);
	writeFileContents("perlin_noise_decompressed.edp", decompressed, decompressed_size);
	delete[] decompressed;
	delete[] compressed;
	
	//compare the two files
	original_size = getFileContents("perlin_noise.edp", &original_data);
	decompressed_size = getFileContents("perlin_noise_decompressed.edp", &decompressed);
	std::cout<<Compare(original_data, original_size, decompressed, decompressed_size);
	delete[] original_data;
	delete[] decompressed;

	//open the decompressed edpc and save it as an image
	ed.import("perlin_noise_decompressed.edp");
	KImage *perlin_noise_km_img = new KImage(dim*sqrt(km), dim*sqrt(km), 8);
	ed.SaveAs(perlin_noise_km_img, sqrt(km));
	perlin_noise_km_img->SaveAs("perlin_noise_km.tif");
	delete perlin_noise_km_img;
	ed = ElementalDust();

	system("pause");
}

int main()
{
	test();
	

	_CrtDumpMemoryLeaks();
	return 0;
}