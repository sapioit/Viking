namespace IO { 
namespace Scheduler {
	template<typename Flags = std::uint8_t>
	struct Context {
		std::unique_ptr<Socket> socket;
		Flags flags;
		Context() = default;
		Context(std::unique_ptr<Socket> socket) : socket(std::move(socket)) {}
		Context(std::unique_ptr<Socket> socket, Flags flags) : socket(std::move(socket)), flags(flags) {}
		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;
		Context(Context&& other) {
			*this = std::move(other);
		}
		Context& operator=(Context& other) {
			if (this != &other) {
		        Close();
		        socket = other.socket;
		        other.socket = nullptr;
		        flags = other.flags;
		    }
		    return *this;
		}
		~Context() {
			Close();
		}
		bool operator=(const Context& other) const {
			return (*(socket->get()) == *(other.socket->get())) && (flags == other.flags);
		}


		void Close() {
			if(socket)
				delete socket;
		}
	};
}}