#ifndef __MINECRAFT__CONSTANTS_HPP
#define __MINECRAFT__CONSTANTS_HPP

// constants are bad but are sometimes needed :-)

constexpr int         CHUNK_X_BLOCK_COUNT = 16;
constexpr int         CHUNK_Y_BLOCK_COUNT = 255;
constexpr int         CHUNK_Z_BLOCK_COUNT = 16;
constexpr float       BLOCK_LENGTH        = 1.f;
constexpr std::size_t TEXTURE_SIDE_LENGTH = 16u; // in pixels

constexpr int RENDER_DISTANCE = 10; // in chunks

#endif // __MINECRAFT__CONSTANTS_HPP