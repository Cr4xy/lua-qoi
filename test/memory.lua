local qoi = require "lua-qoi"
local image = qoi.new(128, 128)
local x, y = image.width / 2, image.height / 2
for a = -math.pi, math.pi, 0.1 do
  local xo = math.floor(math.sin(a) * image.width / 2 * 0.9)
  local yo = math.floor(math.cos(a) * image.height / 2 * 0.9)
  image:setPixel(x + xo, y + yo, 255, 255, 255, 255)
end
local bin = qoi.encode(image)
local bin2 = image:encode()
assert(bin == bin2)
--print("encoded image size", #bin)
local image2 = qoi.decode(bin)
--print("decoded image dimensions", image2.width, image2.height)
assert(image2.width == image.width)
assert(image2.height == image.height)
for angle = -math.pi, math.pi, 0.1 do
  local xo = math.floor(math.sin(angle) * image.width / 2 * 0.9)
  local yo = math.floor(math.cos(angle) * image.height / 2 * 0.9)
  local r, g, b, a = image:getPixel(x + xo, y + yo)
  assert(r == 255 and g == 255 and b == 255 and a == 255)
end
print("memory.lua successful.")