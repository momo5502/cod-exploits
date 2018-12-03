#pragma once

#ifndef STD_INCLUDED
#error "Missing standard header"
#endif

namespace Utils
{
	namespace IO
	{
		template<typename T>
		class Result
		{
		public:
			bool result;
			T data;
		};

		class File
		{
		public:
			File(std::string file);

			Result<uint64_t> size();
			Result<std::string> read();
			bool write(std::string data, bool append = false);
			bool exists();
			bool remove();

			std::string getName(bool withExtension = true);

		private:
			bool createDir();

			std::string path;
		};

		class Directory
		{
		public:
			Directory(std::string dir);

			bool exists();
			bool create();
			bool empty();
			Result<std::vector<std::string>> files();

		private:
			std::string path;
		};
	}
}
