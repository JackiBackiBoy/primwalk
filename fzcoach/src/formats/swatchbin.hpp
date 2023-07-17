#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>

struct SwatchbinModule {
  uint32_t startIndex;
  uint32_t endIndex;

  [[nodiscard]] inline constexpr uint32_t byteCount() const { return endIndex - startIndex + 1; }
};

enum class SwatchbinCompression : int8_t {
  UNKNOWN = -1,
  BC1_UNORM = 71,
  BC3_UNORM = 77,
  BC4_UNORM = 80,
  BC4_SNORM = 81,
  BC5_UNORM = 83,
  BC5_SNORM = 84,
  BC6H_UF16 = 95,
  BC7_UNORM = 98,
  BC7_UNORM_A = 98,
  BC6H_SF16 = 96,
  R8G8B8A8_UNORM = 28,
  A8_UNORM = 65,
  R8_UNORM = 61,
};

inline static const std::unordered_map<int8_t, SwatchbinCompression> swatchbinCompressions = {
  { -1,  SwatchbinCompression::UNKNOWN },
  {  0,  SwatchbinCompression::BC1_UNORM },
  {  2,  SwatchbinCompression::BC3_UNORM },
  {  3,  SwatchbinCompression::BC4_UNORM },
  {  4,  SwatchbinCompression::BC4_SNORM },
  {  5,  SwatchbinCompression::BC5_UNORM },
  {  6,  SwatchbinCompression::BC5_SNORM },
  {  7,  SwatchbinCompression::BC6H_UF16 },
  {  9,  SwatchbinCompression::BC7_UNORM },
  {  22, SwatchbinCompression::BC7_UNORM_A },
  {  12, SwatchbinCompression::BC6H_SF16 },
  {  13, SwatchbinCompression::R8G8B8A8_UNORM },
  {  19, SwatchbinCompression::A8_UNORM },
  {  20, SwatchbinCompression::R8_UNORM },
};

[[nodiscard]] static constexpr bool isBlockType(SwatchbinCompression compression) {
  switch (compression) {
  case SwatchbinCompression::UNKNOWN:
  case SwatchbinCompression::R8G8B8A8_UNORM:
  case SwatchbinCompression::A8_UNORM:
  case SwatchbinCompression::R8_UNORM:
    return false;
    break;
  }

  return true;
}

static constexpr SwatchbinModule SB_WIDTH = { 76, 79 };
static constexpr SwatchbinModule SB_HEIGHT = { 80, 83 };
static constexpr SwatchbinModule SB_COMPRESSION = { 116, 116 };
static constexpr SwatchbinModule SB_MIPMAPCOUNT = { 90, 90 };
static constexpr SwatchbinModule SB_LINEARSIZE = { 128, 131 };
static constexpr SwatchbinModule SB_HEADERLENGTH = { 8, 11 };
static constexpr SwatchbinModule SB_BC16LINEARSIZE = { 188, 191 };

struct BlockCompressionMode {
  uint8_t type;
  uint8_t pBit;
};

BlockCompressionMode extract_mode(const std::vector<uint8_t>& block) {
  BlockCompressionMode mode;

  // The LSB of each block tells us which BC7 mode is in use
  for (size_t i = 0; i < 7; i++) {
    if (block[0] >> (7 - i)) {
      mode.type = i;
      break;
    }
  }

  mode.pBit = (block[0] >> 3) & 0x01; // Extract the p-bit
  //mode.hasAlpha = mode.type > 3;

  return mode;
}

int get_num_subsets(uint8_t type) {
  switch (type) {
  case 0:
  case 2:
    return 3;
  case 1:
  case 3:
  case 7:
    return 2;
  }

  return 1;
}

int getNumPartitionBits(uint8_t type) {
  switch (type) {
  case 0:
    return 4;
  case 1:
  case 2:
  case 3:
  case 7:
    return 6;
  case 4:
  case 5:
  case 6:
    return 0;
  }

  return 0;
}

int getBC7RGBdepth(uint8_t type) {
  switch (type) {
  case 0:
    return 4;
  case 1:
    return 6;
  case 2:
  case 4:
  case 7:
    return 5;
  case 3:
  case 5:
  case 6:
    return 7;
  }

  return 0;
}

int getBC7Channels(uint8_t type) {
  return type < 4 ? 3 : 4;
}

int getNumPBits(uint8_t type) {
  switch (type) {
  case 0:
  case 4:
    return 6;
  case 1:
  case 6:
    return 2;
  case 2:
  case 5:
    return 0;
  case 3:
  case 7:
    return 4;
  }

  return 0;
}

uint8_t extractBitsFromBytes(uint8_t byteA, uint8_t byteB, uint8_t startIndex, uint8_t numBits) {
  uint8_t num = 0;

  uint8_t lhsBits = numBits > (8 - startIndex) ? (8 - startIndex) : numBits;
  uint8_t rhsBits = numBits - lhsBits;

  if (rhsBits == 0) { // partition bits exclusively in first byte
    num = byteA << startIndex;
    num >>= (8 - lhsBits);
  }
  else if (lhsBits == 0) { // partition bits exclusively in second byte
    num = byteB >> (8 - numBits);
  }
  else { // partition bits distributed over two bytes
    num = byteA << startIndex;
    num >>= (8 - lhsBits);
    num |= (byteB >> (8 - rhsBits));
  }

  return num;
}

void decompress_bc7(int x, int y, const std::vector<uint8_t>& block)
{
  BlockCompressionMode mode = extract_mode(block);

  //decode partition data from explicit partition bits
  int subset_index = 0;
  int num_subsets = 1;

  if (mode.type == 0 || mode.type == 1 || mode.type == 2 || mode.type == 3 || mode.type == 7)
  {
    num_subsets = get_num_subsets(mode.type);
    //partition_set_id = extract_partition_set_id(mode, block);
    //subset_index = get_partition_index(num_subsets, partition_set_id, x, y);
  }

  int byteIndex = 0;
  int bitsRead = mode.type + 1; // for example, if it's mode 7, the LSB is a 8-bit identifier

  if (bitsRead % 8 == 0) { // 1 byte read
    byteIndex++;
  }

  /* Partition bits */
  uint8_t partitionBits = getNumPartitionBits(mode.type);
  uint8_t xBitPartition = extractBitsFromBytes(block[0], block[1], bitsRead, partitionBits);
  bitsRead += partitionBits;

  /* Extract endpoint data */
  std::vector<std::vector<uint8_t>> endpoints(2 * num_subsets, std::vector<uint8_t>(4));
  std::vector<uint8_t> endpointPbits(getNumPBits(mode.type));

  uint8_t endpointRGBdepth = getBC7RGBdepth(mode.type);
  uint8_t endpointChannels = getBC7Channels(mode.type);

  bitsRead %= 8;

  // Read RGB(A) endpoint data
  for (size_t r = 0; r < 4; r++) {
    for (size_t c = 0; c < 2 * num_subsets; c++) {
      if (endpointChannels == 4 && r == 3) {
        endpoints[c][r] = 255;
        continue;
      }

      endpoints[c][r] = extractBitsFromBytes(block[bitsRead / 8], block[bitsRead / 8 + 1], bitsRead % 8, endpointRGBdepth);
      bitsRead += endpointRGBdepth;
    }
  }

  // Read endpoint P-bits
  for (size_t i = 0; i < endpointPbits.size(); i++) {
    endpointPbits[i] = block[bitsRead / 8] << ((bitsRead % 8));
    endpointPbits[i] >>= 7;
    bitsRead++;
  }

  /* Decode endpoints */
  if (mode.type == 0 || mode.type == 1 || mode.type == 3 || mode.type == 6 || mode.type == 7) {
    for (size_t i = 0; i < endpoints.size(); i++) {
      endpoints[i][0] <<= 1; // r
      endpoints[i][1] <<= 1; // g
      endpoints[i][2] <<= 1; // b

      if (endpoints[i][3] != 255) {
        endpoints[i][3] <<= 1; // a
      }
    }

    // if P-bit is shared
    if (mode.type == 1) {
      for (size_t i = 0; i < 3; i++) {
        endpoints[0][i] |= endpointPbits[0];
        endpoints[1][i] |= endpointPbits[0];
        endpoints[2][i] |= endpointPbits[1];
        endpoints[3][i] |= endpointPbits[1];
      }
    }
    else { // unique P-bit per endpoint
      for (size_t i = 0; i < endpoints.size(); i++) {
        endpoints[i][0] |= endpointPbits[i]; // r
        endpoints[i][1] |= endpointPbits[i]; // g
        endpoints[i][2] |= endpointPbits[i]; // b

        // TODO: Might break
        endpoints[i][3] |= endpointPbits[i]; // a
      }
    }
  }

  for (size_t i = 0; i < endpoints.size(); i++) {
    //endpoints[i][0] <<= (8 - color_component_precision(mode.type));
  }

  std::cout << "huh" << std::endl;


  ////decode endpoint color and alpha for each subset
  //fully_decode_endpoints(endpoint_array, mode, block);

  ////endpoints are now complete.
  //UINT8 endpoint_start[4] = endpoint_array[2 * subset_index];
  //UINT8 endpoint_end[4] = endpoint_array[2 * subset_index + 1];

  ////Determine the palette index for this pixel
  //alpha_index = get_alpha_index(block, mode, x, y);
  //alpha_bitcount = get_alpha_bitcount(block, mode);
  //color_index = get_color_index(block, mode, x, y);
  //color_bitcount = get_color_bitcount(block, mode);

  ////determine output
  //UINT8 output[4];
  //output.rgb = interpolate(endpoint_start.rgb, endpoint_end.rgb, color_index, color_bitcount);
  //output.a = interpolate(endpoint_start.a, endpoint_end.a, alpha_index, alpha_bitcount);

  //if (mode.type == 4 OR == 5)
  //{
  //  //Decode the 2 color rotation bits as follows:
  //  // 00 � Block format is Scalar(A) Vector(RGB) - no swapping
  //  // 01 � Block format is Scalar(R) Vector(AGB) - swap A and R
  //  // 10 � Block format is Scalar(G) Vector(RAB) - swap A and G
  //  // 11 - Block format is Scalar(B) Vector(RGA) - swap A and B
  //  rotation = extract_rot_bits(mode, block);
  //  output = swap_channels(output, rotation);
  //}

}

void loadSwatchbinFile(const std::string& path) {
  std::string truePath = "C:/Code/primwalk/" + path;
  std::ifstream file(truePath, std::ios_base::binary);

  if (!file.is_open()) {
    std::cout << "Could not open swatchbin file!\n";
    return;
  }

  /* --- File validity checks --- */
  // Header file identifier
  uint32_t fileID;
  file.read(reinterpret_cast<char*>(&fileID), sizeof(uint32_t));

  uint32_t reqID;
  char reqChars[] = "burG";
  memcpy(&reqID, &reqChars, sizeof(uint32_t));

  if (fileID ^ reqID) {
    std::cout << "Header tags are NOT the same" << std::endl;
    file.close();
    return;
  }

  // TXQU

  int8_t compression;
  uint8_t mipmapCount;
  uint32_t dataOffset; // tells the index of beginning of image data
  uint32_t width;
  uint32_t height;
  uint32_t numPixels;
  uint32_t bc16LinearSize;

  file.seekg(SB_COMPRESSION.startIndex);
  file.read(reinterpret_cast<char*>(&compression), SB_COMPRESSION.byteCount());

  file.seekg(SB_MIPMAPCOUNT.startIndex);
  file.read(reinterpret_cast<char*>(&mipmapCount), SB_MIPMAPCOUNT.byteCount());

  file.seekg(SB_HEADERLENGTH.startIndex);
  file.read(reinterpret_cast<char*>(&dataOffset), SB_HEADERLENGTH.byteCount());

  file.seekg(SB_WIDTH.startIndex);
  file.read(reinterpret_cast<char*>(&width), SB_WIDTH.byteCount());

  file.seekg(SB_HEIGHT.startIndex);
  file.read(reinterpret_cast<char*>(&height), SB_HEIGHT.byteCount());

  file.seekg(SB_LINEARSIZE.startIndex);
  file.read(reinterpret_cast<char*>(&numPixels), SB_LINEARSIZE.byteCount());

  file.seekg(SB_BC16LINEARSIZE.startIndex);
  file.read(reinterpret_cast<char*>(&bc16LinearSize), SB_BC16LINEARSIZE.byteCount());

  //swatchWidth = width;
  //swatchHeight = height;
  SwatchbinCompression trueCompression = swatchbinCompressions.at(compression);

  //swatchData = std::vector<unsigned char>(numPixels * 4);

  if (isBlockType(trueCompression)) {
    std::vector<uint8_t> block(16); // 16 byte block
    file.seekg(dataOffset);

    while (file.tellg() != -1) {
      file.read(reinterpret_cast<char*>(block.data()), 16);


      //bc7pixels.resize(numPixels);
      //bc7decomp::unpack_bc7(block.data(), bc7pixels.data());
      //decompress_bc7(0, 0, block);
    }
  }
  else {
    file.seekg(dataOffset); // jump to offset
    std::vector<unsigned char> data(numPixels);
    file.read(reinterpret_cast<char*>(data.data()), numPixels);

    for (size_t i = 0; i < numPixels; i++) {
      //swatchData[i] = data[i];
    }
  }

  //for (size_t i = 0; i < bc7pixels.size(); i++) {
  //  swatchData[i] = bc7pixels[i].r;
  //  swatchData[i + 1] = bc7pixels[i].g;
  //  swatchData[i + 2] = bc7pixels[i].b;
  //  swatchData[i + 3] = bc7pixels[i].a;
  //}

  std::cout << "Swatchbin file read complete\n";

  file.close();
}