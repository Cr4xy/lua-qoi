local qoi = require "lua-qoi"
local image = qoi.new(128, 128, 3)
local x, y = image.width / 2, image.height / 2
for a = -math.pi, math.pi, 0.1 do
  local xo = math.floor(math.sin(a) * image.width / 2 * 0.9)
  local yo = math.floor(math.cos(a) * image.height / 2 * 0.9)
  image:setPixel(x + xo, y + yo, 255, 255, 255)
end
qoi.write(image, "circle-no-alpha.qoi")
os.remove("circle-no-alpha.qoi")
print("circle-no-alpha.lua successful.")