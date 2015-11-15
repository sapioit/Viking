#ifndef READER
#define READER

int getpagesize(void);

namespace IO
{
template <typename Socket, typename DataType> class SocketReader
{
	const Socket *socket = nullptr;
	SocketReader() = default;

	public:
	SocketReader(const Socket &sock) : socket(&sock) {}
	virtual ~SocketReader() = default;

	DataType operator()(std::size_t max_size = 0) const
	{
		if (socket != nullptr) {
			auto to_read = max_size == 0 ? getpagesize() : max_size;
			DataType buffer(to_read);
			auto read = socket->ReadSome(buffer);
			buffer.resize(read);
			return buffer;
		}
		return {};
	}
};
}
#endif // READER
