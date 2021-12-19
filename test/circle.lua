local qoi = require "lua-qoi"
local image = qoi.new(128, 128)
local x, y = image.width / 2, image.height / 2
for a = -math.pi, math.pi, 0.3 do
  local xo = math.floor(math.sin(a) * image.width / 2 * 0.9)
  local yo = math.floor(math.cos(a) * image.height / 2 * 0.9)
  image:setPixel(x + xo, y + yo, 255, 255, 255)

  a = a + 0.1
  xo = math.floor(math.sin(a) * image.width / 2 * 0.9)
  yo = math.floor(math.cos(a) * image.height / 2 * 0.9)
  image:setPixel(x + xo, y + yo, 255, 0, 0, 255)

  a = a + 0.1
  xo = math.floor(math.sin(a) * image.width / 2 * 0.9)
  yo = math.floor(math.cos(a) * image.height / 2 * 0.9)
  image:setPixel(x + xo, y + yo, 255, 255, 0, 128)
end
qoi.write(image, "circle.qoi")
os.remove("circle.qoi")
print("circle.lua successful.")