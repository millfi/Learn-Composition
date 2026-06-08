#ifndef TEXTLINE_CPP
#define TEXTLINE_CPP

#include <pch.h>
#include <vector>

#endif
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::UI;
using std::vector;
// テキスト描画のための直線状に並んだランダムな形の長方形
struct Lined_Rect
{
	Lined_Rect(winrt::Windows::UI::Composition::Compositor const& compositor, vector<float> const& width, vector<float> const& height, vector<float3> const& offset)
	{
		assert(width.size() == height.size() && height.size() == offset.size());
		for (size_t i = 0; i < width.size(); ++i){
			auto box = compositor.CreateSpriteVisual();
			box.Size({ width[i], height[i] });
			box.Offset(offset[i]);
			box.Brush(
				compositor.CreateColorBrush(
					Color{ 255, static_cast<uint8_t>(rand() % 256), 
								static_cast<uint8_t>(rand() % 256), 
								static_cast<uint8_t>(rand() % 256) 
					}
				)
			);
			m_boxes.push_back(box);
		}
	}
	vector<winrt::Windows::UI::Composition::SpriteVisual> m_boxes;
};
struct TextLine
{
	vector<Lined_Rect> lines;
};
