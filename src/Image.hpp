#ifndef __MINECRAFT__IMAGE_HPP
#define __MINECRAFT__IMAGE_HPP

#include "Pch.hpp"
#include "Vector.hpp"
#include "ErrorHandler.hpp"

class Image {
private:
    std::unique_ptr<Coloru8[]> m_pBuffer;

    UINT m_width, m_height, m_nPixels;

public:
    inline Image() noexcept = default;

    Image(const wchar_t* filename) noexcept {
        Microsoft::WRL::ComPtr<IWICBitmapSource>      decodedConvertedFrame;
		Microsoft::WRL::ComPtr<IWICBitmapDecoder>     bitmapDecoder;
		Microsoft::WRL::ComPtr<IWICImagingFactory>    factory;
		Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frameDecoder;

		if (CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)))
			FATAL_ERROR("Failed to create IWICImagingFactory");

		if (factory->CreateDecoderFromFilename(filename, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &bitmapDecoder))
			FATAL_ERROR("Failed to CreateDecoderFromFilename");

		if (bitmapDecoder->GetFrame(0, &frameDecoder))
			FATAL_ERROR("Failed to GetFrame the first frame of an image");

		if (frameDecoder->GetSize((UINT*)&this->m_width, (UINT*)&this->m_height))
			FATAL_ERROR("Failed to GetSize of an image");

		this->m_nPixels = this->m_width * this->m_height;
		if (WICConvertBitmapSource(GUID_WICPixelFormat32bppRGBA, frameDecoder.Get(), &decodedConvertedFrame))
			FATAL_ERROR("Failed to WICConvertBitmapSource");

		this->m_pBuffer = std::make_unique<Coloru8[]>(this->m_nPixels * sizeof(Coloru8));
		const WICRect sampleRect{ 0, 0, static_cast<INT>(this->m_width), static_cast<INT>(this->m_height) };
		if (decodedConvertedFrame->CopyPixels(&sampleRect, this->m_width * sizeof(Coloru8), this->m_nPixels * sizeof(Coloru8), (BYTE*)this->m_pBuffer.get()))
			FATAL_ERROR("Failed to CopyPixels from an image");
    }

    inline UINT     GetWidth()         const noexcept { return this->m_width;         }
    inline UINT     GetHeight()        const noexcept { return this->m_height;        }
    inline UINT     GetPixelCount()    const noexcept { return this->m_nPixels;       }
    inline Coloru8* GetBufferPointer() const noexcept { return this->m_pBuffer.get(); }
};

#endif // __MINECRAFT__IMAGE_HPP