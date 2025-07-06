#pragma once

#include "Util.h"

#include <string>

namespace serialize
{
	class Reader
	{
	public:
		explicit Reader(std::string_view buffer) :
			m_buffer(buffer),
			m_pos(0)
		{}

		template<class T>
		void Read(T& object)
		{
			std::string_view data = m_buffer.substr(m_pos, sizeof(T));

			object = *reinterpret_cast<const T*>(data.data());

			m_pos += sizeof(T);
		}

	private:
		std::string_view m_buffer;
		size_t m_pos;
	};

	class Writer
	{
	public:
		explicit Writer(std::string& buffer) :
			m_buffer(buffer),
			m_pos(0)
		{}

		template<class T>
		void Write(const T& object)
		{
			m_buffer += ToData(object);
			m_pos += sizeof(T);
		}

	private:
		std::string& m_buffer;
		size_t m_pos;
	};
}