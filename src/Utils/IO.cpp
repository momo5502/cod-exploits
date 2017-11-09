#include "STDInclude.hpp"

namespace Utils
{
	namespace IO
	{
		File::File(std::string file) : path(file) {}

		Result<uint64_t> File::size()
		{
			Result<uint64_t> result;
			result.result = false;

			std::ifstream stream(this->path);
			if (stream.good())
			{
				result.result = true;
				stream.seekg(0, std::ios::end);
				result.data = stream.tellg();
			}

			return result;
		}

		Result<std::string> File::read()
		{
			Result<std::string> result;
			result.result = false;

			std::ifstream stream(this->path, std::ios::binary);
			if (stream.is_open() && stream.good())
			{
				stream.seekg(0, std::ios::end);
				std::streamsize size = stream.tellg();
				stream.seekg(0, std::ios::beg);

				if (size > -1)
				{
					result.result = true;
					result.data.resize(static_cast<uint32_t>(size));
					stream.read(const_cast<char*>(result.data.data()), size);
					stream.close();
				}
			}

			return result;
		}

		bool File::write(std::string data, bool append)
		{
			this->createDir();

			std::ofstream stream(this->path, std::ios::binary | std::ofstream::out | (append ? std::ofstream::app : std::ofstream::out));
			if (!stream.is_open()) return false;

			stream.write(data.data(), data.size());
			stream.close();
			return true;
		}

		bool File::exists()
		{
			return std::ifstream(this->path).good();
		}

		bool File::remove()
		{
			return ::remove(this->path.data()) == 0;
		}

		bool File::createDir()
		{
			std::string dir = this->path;

			auto pos = dir.find_last_of("/\\");
			if (pos != std::string::npos)
			{
				dir = dir.substr(0, pos);

				return Directory(dir).create();
			}

			return true;
		}

		std::string File::getName(bool withExtension)
		{
			std::string _path = this->path;
			auto pos = _path.find_last_of("/\\");
			if (pos != std::string::npos) _path = _path.substr(pos + 1);

			if (!withExtension)
			{
				pos = _path.find_last_of(".");
				if (pos != std::string::npos) _path = _path.substr(0, pos);
			}

			return _path;
		}

		Directory::Directory(std::string dir) : path(dir) {}

		bool Directory::exists()
		{
			return std::experimental::filesystem::is_directory(this->path);
		}

		bool Directory::create()
		{
			return std::experimental::filesystem::create_directories(this->path);
		}

		bool Directory::empty()
		{
			return std::experimental::filesystem::is_empty(this->path);
		}

		Result<std::vector<std::string>> Directory::files()
		{
			Result<std::vector<std::string>> result;
			result.result = this->exists();

			if (result.result)
			{
				for (auto& file : std::experimental::filesystem::directory_iterator(this->path))
				{
					result.data.push_back(file.path().generic_string());
				}
			}

			return result;
		}
	}
}
