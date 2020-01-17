ngx.req.read_body()
local arg = ngx.req.get_post_args()
local exec = require'resty.exec'
local prog = exec.new('/tmp/exec.sock')
local ip = ngx.var.remote_addr
local reset
local chinaroute
local timeout
if arg.reset then
	reset = arg.reset
else
	reset = "noreset"
end
if arg.chinaroute then
	chinaroute = arg.chinaroute
else
	chinaroute = 0
end
if arg.timeout then
	timeout = arg.timeout
else
	timeout = 0
end
local res, err = prog( "/wlt", ip, reset, "/etc/wlt.conf", chinaroute, 0, 0, timeout)
--   <wlt_program> <ip> <reset> <conffile> <route1> <route2> <route3> <timeout>
ngx.say(res.stdout)
