#pragma once
#include "CommonInclude.h"
#include "asMath.h"
namespace as
{
	struct asColor
	{
		uint32_t rgba = 0;

		constexpr asColor(uint32_t rgba) :rgba(rgba) {}
		constexpr asColor(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255) : rgba((r << 0) | (g << 8) | (b << 16) | (a << 24)) {}

		constexpr uint8_t getR() const { return (rgba >> 0) & 0xFF; }
		constexpr uint8_t getG() const { return (rgba >> 8) & 0xFF; }
		constexpr uint8_t getB() const { return (rgba >> 16) & 0xFF; }
		constexpr uint8_t getA() const { return (rgba >> 24) & 0xFF; }

		constexpr void setR(uint8_t value) { *this = asColor(value, getG(), getB(), getA()); }
		constexpr void setG(uint8_t value) { *this = asColor(getR(), value, getB(), getA()); }
		constexpr void setB(uint8_t value) { *this = asColor(getR(), getG(), value, getA()); }
		constexpr void setA(uint8_t value) { *this = asColor(getR(), getG(), getB(), value); }

		constexpr XMFLOAT3 toFloat3() const
		{
			return XMFLOAT3(
				((rgba >> 0) & 0xFF) / 255.0f,
				((rgba >> 8) & 0xFF) / 255.0f,
				((rgba >> 16) & 0xFF) / 255.0f
			);
		}
		constexpr XMFLOAT4 toFloat4() const
		{
			return XMFLOAT4(
				((rgba >> 0) & 0xFF) / 255.0f,
				((rgba >> 8) & 0xFF) / 255.0f,
				((rgba >> 16) & 0xFF) / 255.0f,
				((rgba >> 24) & 0xFF) / 255.0f
			);
		}

		static constexpr asColor fromFloat4(const XMFLOAT4& value)
		{
			return asColor((uint8_t)(value.x * 255), (uint8_t)(value.y * 255), (uint8_t)(value.z * 255), (uint8_t)(value.w * 255));
		}
		static constexpr asColor fromFloat3(const XMFLOAT3& value)
		{
			return asColor((uint8_t)(value.x * 255), (uint8_t)(value.y * 255), (uint8_t)(value.z * 255));
		}

		static constexpr asColor lerp(asColor a, asColor b, float i)
		{
			return fromFloat4(as::asMath::Lerp(a.toFloat4(), b.toFloat4(), i));
		}

		static constexpr asColor Red() { return asColor(255, 0, 0, 255); }
		static constexpr asColor Green() { return asColor(0, 255, 0, 255); }
		static constexpr asColor Blue() { return asColor(0, 0, 255, 255); }
		static constexpr asColor Black() { return asColor(0, 0, 0, 255); }
		static constexpr asColor White() { return asColor(255, 255, 255, 255); }
		static constexpr asColor Yellow() { return asColor(255, 255, 0, 255); }
		static constexpr asColor Purple() { return asColor(255, 0, 255, 255); }
		static constexpr asColor Cyan() { return asColor(0, 255, 255, 255); }
		static constexpr asColor Transparent() { return asColor(0, 0, 0, 0); }
		static constexpr asColor Gray() { return asColor(127, 127, 127, 255); }
		static constexpr asColor Ghost() { return asColor(127, 127, 127, 127); }
		static constexpr asColor Booger() { return asColor(127, 127, 127, 200); }
	};
}


