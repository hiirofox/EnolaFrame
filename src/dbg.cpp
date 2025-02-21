#include "dbg.h"

void HResultCheckFailed(HRESULT hr) {
	if (FAILED(hr))
	{
#if defined(_DEBUG)
		_com_error err(hr);
		printf("%s\n", err.ErrorMessage());
#endif
		while (1);
	}
}

void DBG(const char* format, ...) {
#if defined(_DEBUG)
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
#endif
}