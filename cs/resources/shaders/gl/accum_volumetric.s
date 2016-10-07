function normal		(shader, t_base, t_second, t_detail)
	shader:begin	("accum_volumetric", "accum_volumetric")
			: fog		(false)
			: zb 		(true,false)
			: blend		(true,blend.one,blend.one)
--			: aref 		(true,0)
			: sorting	(2, false)
--	TODO: Implement sampler states
	shader:sampler	("s_lmap")      :texture	(t_base): clamp()
	shader:sampler	("s_smap")      :texture	("$user$smap_depth") : comp_less ()
	shader:sampler	("s_noise")    	:texture("fx\\fx_noise")	: f_linear ()
end