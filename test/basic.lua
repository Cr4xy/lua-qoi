local qoi = require "lua-qoi"
local image = qoi.read("../images/lua_qoi.qoi")
local x, y = image.width / 2, image.height / 2
image:setPixel(x, y, 255, 255, 255, 255)
image:write("test.qoi")
os.remove("test.qoi")
print("basic.lua successful.")