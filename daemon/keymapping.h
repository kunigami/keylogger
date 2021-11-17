#include <string>

#include <CoreGraphics/CoreGraphics.h>

#define LRMOD_ALT   0
#define LRMOD_CMD   6
#define LRMOD_CTRL  9
#define LRMOD_SHIFT 3
#define LMOD_OFFS   1
#define RMOD_OFFS   2

bool get_is_shift(uint32_t flag);
bool get_is_cmd(uint32_t flag);
bool get_is_ctrl(uint32_t flag);

std::string key_code_to_str(int keyCode, bool shift, bool caps);
