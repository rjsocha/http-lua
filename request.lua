local function printf(...) return io.stdout:write(string.format(...)) end

local result = {
response = "Success on " .. request.url .. "\n",
code = 200
}

printf("URL: %s\n",request.url)
printf("METHOD: %s\n",request.method)

if request.headers["host"] ~= nil then
  print("HOST: " .. request.headers["host"])
end

-- Dump headers
printf("HEADERS:\n")
for k,v in pairs(request.headers) do
  printf("%s: %s\n",k,v)
end

return result
