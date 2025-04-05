from .datatypes import Array, Float, Int, Pointer, String, Struct, TextureHandle


class Sound(Struct):
	def __init__(self, filename=""):
		Struct.__init__(self, "CDataSound")
		self.id = Int(0)
		self.filename = String(filename)


class SoundSet(Struct):
	def __init__(self, name="", files=()):
		Struct.__init__(self, "CDataSoundset")
		self.name = String(name)
		self.sounds = Array(Sound())
		self.last = Int(-1)
		for filename in files:
			self.sounds.add(Sound(filename))


class Image(Struct):
	def __init__(self, name="", filename="", linear_mapping=0):
		Struct.__init__(self, "CDataImage")
		self.name = String(name)
		self.filename = String(filename)
		self.flag = Int(linear_mapping)
		self.id = TextureHandle()


class SpriteSet(Struct):
	def __init__(self, _name="", image=None, gridx=0, gridy=0):
		Struct.__init__(self, "CDataSpriteset")
		self.image = Pointer(Image, image)  # TODO
		self.gridx = Int(gridx)
		self.gridy = Int(gridy)


class Sprite(Struct):
	def __init__(self, name="", Set=None, x=0, y=0, w=0, h=0):
		Struct.__init__(self, "CDataSprite")
		self.name = String(name)
		self.set = Pointer(SpriteSet, Set)  # TODO
		self.x = Int(x)
		self.y = Int(y)
		self.w = Int(w)
		self.h = Int(h)


class Pickup(Struct):
	def __init__(self, name="", respawntime=15, spawndelay=0):
		Struct.__init__(self, "CDataPickupspec")
		self.name = String(name)
		self.respawntime = Int(respawntime)
		self.spawndelay = Int(spawndelay)


class AnimKeyframe(Struct):
	def __init__(self, time=0, x=0, y=0, angle=0):
		Struct.__init__(self, "CAnimKeyframe")
		self.time = Float(time)
		self.x = Float(x)
		self.y = Float(y)
		self.angle = Float(angle)


class AnimSequence(Struct):
	def __init__(self):
		Struct.__init__(self, "CAnimSequence")
		self.frames = Array(AnimKeyframe())


class Animation(Struct):
	def __init__(self, name=""):
		Struct.__init__(self, "CAnimation")
		self.name = String(name)
		self.body = AnimSequence()
		self.back_foot = AnimSequence()
		self.front_foot = AnimSequence()
		self.attach = AnimSequence()


class WeaponSpec(Struct):
	def __init__(self, cont=None, name=""):
		Struct.__init__(self, "CDataWeaponspec")
		self.name = String(name)
		self.sprite_body = Pointer(Sprite, Sprite())
		self.sprite_cursor = Pointer(Sprite, Sprite())
		self.sprite_proj = Pointer(Sprite, Sprite())
		self.sprite_muzzles = Array(Pointer(Sprite, Sprite()))
		self.visual_size = Int(96)

		self.firedelay = Int(500)
		self.maxammo = Int(10)
		self.ammoregentime = Int(0)
		self.damage = Int(1)

		self.offsetx = Float(0)
		self.offsety = Float(0)
		self.muzzleoffsetx = Float(0)
		self.muzzleoffsety = Float(0)
		self.muzzleduration = Float(5)

		# dig out sprites if we have a container
		if cont:
			for sprite in cont.sprites.items:
				if sprite.name.value == f"weapon_{name}_body":
					self.sprite_body.set_value(sprite)
				elif sprite.name.value == f"weapon_{name}_cursor":
					self.sprite_cursor.set_value(sprite)
				elif sprite.name.value == f"weapon_{name}_proj":
					self.sprite_proj.set_value(sprite)
				elif f"weapon_{name}_muzzle" in sprite.name.value:
					self.sprite_muzzles.add(Pointer(Sprite, sprite))


class Weapon_Hammer(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecHammer")
		self.base = Pointer(WeaponSpec, WeaponSpec())


class Weapon_Gun(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecGun")
		self.base = Pointer(WeaponSpec, WeaponSpec())
		self.curvature = Float(1.25)
		self.speed = Float(2200)
		self.lifetime = Float(2.0)


class Weapon_Shotgun(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecShotgun")
		self.base = Pointer(WeaponSpec, WeaponSpec())
		self.curvature = Float(1.25)
		self.speed = Float(2200)
		self.speeddiff = Float(0.8)
		self.lifetime = Float(0.25)


class Weapon_Grenade(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecGrenade")
		self.base = Pointer(WeaponSpec, WeaponSpec())
		self.curvature = Float(7.0)
		self.speed = Float(1000)
		self.lifetime = Float(2.0)


class Weapon_Laser(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecLaser")
		self.base = Pointer(WeaponSpec, WeaponSpec())
		self.reach = Float(800.0)
		self.bounce_delay = Int(150)
		self.bounce_num = Int(1)
		self.bounce_cost = Float(0)


class Weapon_Ninja(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecNinja")
		self.base = Pointer(WeaponSpec, WeaponSpec())
		self.duration = Int(15000)
		self.movetime = Int(200)
		self.velocity = Int(50)


class Weapons(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataWeaponspecs")
		self.hammer = Weapon_Hammer()
		self.gun = Weapon_Gun()
		self.shotgun = Weapon_Shotgun()
		self.grenade = Weapon_Grenade()
		self.laser = Weapon_Laser()
		self.ninja = Weapon_Ninja()
		self.id = Array(WeaponSpec())


class Explosion(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataExplosion")
		self.radius = Float(135)
		self.max_force = Float(12)


class DataContainer(Struct):
	def __init__(self):
		Struct.__init__(self, "CDataContainer")
		self.sounds = Array(SoundSet())
		self.images = Array(Image())
		self.pickups = Array(Pickup())
		self.spritesets = Array(SpriteSet())
		self.sprites = Array(Sprite())
		self.animations = Array(Animation())
		self.weapons = Weapons()


def FileList(fmt, num):
	return [fmt % (x + 1) for x in range(0, num)]


container = DataContainer()
container.sounds.add(SoundSet("gun_fire", FileList("audio/wp_gun_fire-%02d.wv", 3)))
container.sounds.add(SoundSet("shotgun_fire", FileList("audio/wp_shotty_fire-%02d.wv", 3)))

container.sounds.add(SoundSet("grenade_fire", FileList("audio/wp_flump_launch-%02d.wv", 3)))
container.sounds.add(SoundSet("hammer_fire", FileList("audio/wp_hammer_swing-%02d.wv", 3)))
container.sounds.add(SoundSet("hammer_hit", FileList("audio/wp_hammer_hit-%02d.wv", 3)))
container.sounds.add(SoundSet("ninja_fire", FileList("audio/wp_ninja_attack-%02d.wv", 3)))
container.sounds.add(SoundSet("grenade_explode", FileList("audio/wp_flump_explo-%02d.wv", 3)))
container.sounds.add(SoundSet("ninja_hit", FileList("audio/wp_ninja_hit-%02d.wv", 3)))
container.sounds.add(SoundSet("laser_fire", FileList("audio/wp_laser_fire-%02d.wv", 3)))
container.sounds.add(SoundSet("laser_bounce", FileList("audio/wp_laser_bnce-%02d.wv", 3)))
container.sounds.add(SoundSet("weapon_switch", FileList("audio/wp_switch-%02d.wv", 3)))

container.sounds.add(SoundSet("player_pain_short", FileList("audio/vo_teefault_pain_short-%02d.wv", 12)))
container.sounds.add(SoundSet("player_pain_long", FileList("audio/vo_teefault_pain_long-%02d.wv", 2)))

container.sounds.add(SoundSet("body_land", FileList("audio/foley_land-%02d.wv", 4)))
container.sounds.add(SoundSet("player_airjump", FileList("audio/foley_dbljump-%02d.wv", 3)))
container.sounds.add(SoundSet("player_jump", FileList("audio/foley_foot_left-%02d.wv", 4) + FileList("audio/foley_foot_right-%02d.wv", 4)))
container.sounds.add(SoundSet("player_die", FileList("audio/foley_body_splat-%02d.wv", 3)))
container.sounds.add(SoundSet("player_spawn", FileList("audio/vo_teefault_spawn-%02d.wv", 7)))
container.sounds.add(SoundSet("player_skid", FileList("audio/sfx_skid-%02d.wv", 4)))
container.sounds.add(SoundSet("tee_cry", FileList("audio/vo_teefault_cry-%02d.wv", 2)))

container.sounds.add(SoundSet("hook_loop", FileList("audio/hook_loop-%02d.wv", 2)))

container.sounds.add(SoundSet("hook_attach_ground", FileList("audio/hook_attach-%02d.wv", 3)))
container.sounds.add(SoundSet("hook_attach_player", FileList("audio/foley_body_impact-%02d.wv", 3)))
container.sounds.add(SoundSet("hook_noattach", FileList("audio/hook_noattach-%02d.wv", 2)))
container.sounds.add(SoundSet("pickup_health", FileList("audio/sfx_pickup_hrt-%02d.wv", 2)))
container.sounds.add(SoundSet("pickup_armor", FileList("audio/sfx_pickup_arm-%02d.wv", 4)))

container.sounds.add(SoundSet("pickup_grenade", ["audio/sfx_pickup_launcher.wv"]))
container.sounds.add(SoundSet("pickup_shotgun", ["audio/sfx_pickup_sg.wv"]))
container.sounds.add(SoundSet("pickup_ninja", ["audio/sfx_pickup_ninja.wv"]))
container.sounds.add(SoundSet("weapon_spawn", FileList("audio/sfx_spawn_wpn-%02d.wv", 3)))
container.sounds.add(SoundSet("weapon_noammo", FileList("audio/wp_noammo-%02d.wv", 5)))

container.sounds.add(SoundSet("hit", FileList("audio/sfx_hit_weak-%02d.wv", 2)))

container.sounds.add(SoundSet("chat_server", ["audio/sfx_msg-server.wv"]))
container.sounds.add(SoundSet("chat_client", ["audio/sfx_msg-client.wv"]))
container.sounds.add(SoundSet("chat_highlight", ["audio/sfx_msg-highlight.wv"]))
container.sounds.add(SoundSet("ctf_drop", ["audio/sfx_ctf_drop.wv"]))
container.sounds.add(SoundSet("ctf_return", ["audio/sfx_ctf_rtn.wv"]))
container.sounds.add(SoundSet("ctf_grab_pl", ["audio/sfx_ctf_grab_pl.wv"]))
container.sounds.add(SoundSet("ctf_grab_en", ["audio/sfx_ctf_grab_en.wv"]))
container.sounds.add(SoundSet("ctf_capture", ["audio/sfx_ctf_cap_pl.wv"]))

container.sounds.add(SoundSet("menu", ["audio/music_menu.wv"]))

image_null = Image("null", "")
image_particles = Image("particles", "particles.png")
image_game = Image("game", "game.png")
image_browseicons = Image("browseicons", "ui/icons/browse.png", 1)
image_browsericon = Image("browser", "ui/icons/browser.png", 1)
image_emoticons = Image("emoticons", "emoticons.png")
image_demobuttons = Image("demobuttons", "ui/demo_buttons.png", 1)
image_fileicons = Image("fileicons", "ui/file_icons.png", 1)
image_guibuttons = Image("guibuttons", "ui/gui_buttons.png", 1)
image_guiicons = Image("guiicons", "ui/gui_icons.png", 1)
image_menuicons = Image("menuicons", "ui/icons/menu.png", 1)
image_soundicons = Image("soundicons", "ui/sound_icons.png", 1)
image_toolicons = Image("toolicons", "ui/icons/tools.png", 1)
image_arrowicons = Image("arrowicons", "ui/icons/arrows.png", 1)
image_friendicons = Image("friendicons", "ui/icons/friend.png", 1)
image_levelicons = Image("levelicons", "ui/icons/level.png", 1)
image_sidebaricons = Image("sidebaricons", "ui/icons/sidebar.png", 1)
image_chatwhisper = Image("chatwhisper", "ui/icons/chat_whisper.png", 1)
image_timerclock = Image("timerclock", "ui/icons/timer_clock.png", 1)

container.images.add(image_null)
container.images.add(image_game)
container.images.add(Image("deadtee", "deadtee.png"))
container.images.add(image_particles)
container.images.add(Image("cursor", "ui/gui_cursor.png"))
container.images.add(Image("banner", "ui/gui_logo.png"))
container.images.add(image_emoticons)
container.images.add(image_browseicons)
container.images.add(image_browsericon)
container.images.add(Image("console_bg", "ui/console.png"))
container.images.add(Image("console_bar", "ui/console_bar.png"))
container.images.add(image_demobuttons)
container.images.add(image_fileicons)
container.images.add(image_guibuttons)
container.images.add(image_guiicons)
container.images.add(Image("no_skinpart", "ui/no_skinpart.png"))
container.images.add(image_menuicons)
container.images.add(image_soundicons)
container.images.add(image_toolicons)
container.images.add(image_arrowicons)
container.images.add(image_friendicons)
container.images.add(image_levelicons)
container.images.add(image_sidebaricons)
container.images.add(image_chatwhisper)
container.images.add(Image("raceflag", "race_flag.png"))
container.images.add(image_timerclock)

container.pickups.add(Pickup("health"))
container.pickups.add(Pickup("armor"))
container.pickups.add(Pickup("grenade"))
container.pickups.add(Pickup("shotgun"))
container.pickups.add(Pickup("laser"))
container.pickups.add(Pickup("ninja", 90, 90))
container.pickups.add(Pickup("gun"))
container.pickups.add(Pickup("hammer"))

set_particles = SpriteSet("particles", image_particles, 8, 8)
set_game = SpriteSet("game", image_game, 32, 16)
set_tee_body = SpriteSet("tee_body", image_null, 2, 2)
set_tee_markings = SpriteSet("tee_markings", image_null, 1, 1)
set_tee_decoration = SpriteSet("tee_decoration", image_null, 2, 1)
set_tee_hands = SpriteSet("tee_hands", image_null, 2, 1)
set_tee_feet = SpriteSet("tee_feet", image_null, 2, 1)
set_tee_eyes = SpriteSet("tee_eyes", image_null, 2, 4)
set_tee_hats = SpriteSet("tee_hats", image_null, 1, 4)
set_tee_bot = SpriteSet("tee_bot", image_null, 12, 5)
set_browseicons = SpriteSet("browseicons", image_browseicons, 4, 2)
set_browsericon = SpriteSet("browsericon", image_browsericon, 1, 2)
set_emoticons = SpriteSet("emoticons", image_emoticons, 4, 4)
set_demobuttons = SpriteSet("demobuttons", image_demobuttons, 5, 1)
set_fileicons = SpriteSet("fileicons", image_fileicons, 8, 1)
set_guibuttons = SpriteSet("guibuttons", image_guibuttons, 12, 4)
set_guiicons = SpriteSet("guiicons", image_guiicons, 8, 2)
set_menuicons = SpriteSet("menuicons", image_menuicons, 4, 4)
set_toolicons = SpriteSet("toolicons", image_toolicons, 4, 2)
set_soundicons = SpriteSet("guiicons", image_soundicons, 1, 2)
set_arrowicons = SpriteSet("arrowicons", image_arrowicons, 4, 3)
set_friendicons = SpriteSet("friendicons", image_friendicons, 2, 2)
set_levelicons = SpriteSet("levelicons", image_levelicons, 4, 4)
set_sidebaricons = SpriteSet("sidebaricons", image_sidebaricons, 4, 2)
set_timerclock = SpriteSet("timerclock", image_timerclock, 1, 2)

container.spritesets.add(set_particles)
container.spritesets.add(set_game)
container.spritesets.add(set_tee_body)
container.spritesets.add(set_tee_markings)
container.spritesets.add(set_tee_decoration)
container.spritesets.add(set_tee_hands)
container.spritesets.add(set_tee_feet)
container.spritesets.add(set_tee_eyes)
container.spritesets.add(set_tee_hats)
container.spritesets.add(set_tee_bot)
container.spritesets.add(set_browseicons)
container.spritesets.add(set_emoticons)
container.spritesets.add(set_demobuttons)
container.spritesets.add(set_fileicons)
container.spritesets.add(set_guibuttons)
container.spritesets.add(set_guiicons)
container.spritesets.add(set_menuicons)
container.spritesets.add(set_soundicons)
container.spritesets.add(set_toolicons)
container.spritesets.add(set_arrowicons)
container.spritesets.add(set_friendicons)
container.spritesets.add(set_levelicons)
container.spritesets.add(set_sidebaricons)
container.spritesets.add(set_timerclock)
container.spritesets.add(set_browsericon)

container.sprites.add(Sprite("part_slice", set_particles, 0, 0, 1, 1))
container.sprites.add(Sprite("part_ball", set_particles, 1, 0, 1, 1))
container.sprites.add(Sprite("part_splat01", set_particles, 2, 0, 1, 1))
container.sprites.add(Sprite("part_splat02", set_particles, 3, 0, 1, 1))
container.sprites.add(Sprite("part_splat03", set_particles, 4, 0, 1, 1))

container.sprites.add(Sprite("part_smoke", set_particles, 0, 1, 1, 1))
container.sprites.add(Sprite("part_shell", set_particles, 0, 2, 2, 2))
container.sprites.add(Sprite("part_expl01", set_particles, 0, 4, 4, 4))
container.sprites.add(Sprite("part_airjump", set_particles, 2, 2, 2, 2))
container.sprites.add(Sprite("part_hit01", set_particles, 4, 1, 2, 2))

container.sprites.add(Sprite("health_full", set_game, 21, 0, 2, 2))
container.sprites.add(Sprite("health_empty", set_game, 23, 0, 2, 2))
container.sprites.add(Sprite("armor_full", set_game, 21, 2, 2, 2))
container.sprites.add(Sprite("armor_empty", set_game, 23, 2, 2, 2))

container.sprites.add(Sprite("star1", set_game, 15, 0, 2, 2))
container.sprites.add(Sprite("star2", set_game, 17, 0, 2, 2))
container.sprites.add(Sprite("star3", set_game, 19, 0, 2, 2))

container.sprites.add(Sprite("part1", set_game, 6, 0, 1, 1))
container.sprites.add(Sprite("part2", set_game, 6, 1, 1, 1))
container.sprites.add(Sprite("part3", set_game, 7, 0, 1, 1))
container.sprites.add(Sprite("part4", set_game, 7, 1, 1, 1))
container.sprites.add(Sprite("part5", set_game, 8, 0, 1, 1))
container.sprites.add(Sprite("part6", set_game, 8, 1, 1, 1))
container.sprites.add(Sprite("part7", set_game, 9, 0, 2, 2))
container.sprites.add(Sprite("part8", set_game, 11, 0, 2, 2))
container.sprites.add(Sprite("part9", set_game, 13, 0, 2, 2))

container.sprites.add(Sprite("weapon_gun_body", set_game, 2, 4, 4, 2))
container.sprites.add(Sprite("weapon_gun_cursor", set_game, 0, 4, 2, 2))
container.sprites.add(Sprite("weapon_gun_proj", set_game, 6, 4, 2, 2))
container.sprites.add(Sprite("weapon_gun_muzzle1", set_game, 8, 4, 4, 2))
container.sprites.add(Sprite("weapon_gun_muzzle2", set_game, 12, 4, 4, 2))
container.sprites.add(Sprite("weapon_gun_muzzle3", set_game, 16, 4, 4, 2))

container.sprites.add(Sprite("weapon_shotgun_body", set_game, 2, 6, 8, 2))
container.sprites.add(Sprite("weapon_shotgun_cursor", set_game, 0, 6, 2, 2))
container.sprites.add(Sprite("weapon_shotgun_proj", set_game, 10, 6, 2, 2))
container.sprites.add(Sprite("weapon_shotgun_muzzle1", set_game, 12, 6, 4, 2))
container.sprites.add(Sprite("weapon_shotgun_muzzle2", set_game, 16, 6, 4, 2))
container.sprites.add(Sprite("weapon_shotgun_muzzle3", set_game, 20, 6, 4, 2))

container.sprites.add(Sprite("weapon_grenade_body", set_game, 2, 8, 7, 2))
container.sprites.add(Sprite("weapon_grenade_cursor", set_game, 0, 8, 2, 2))
container.sprites.add(Sprite("weapon_grenade_proj", set_game, 10, 8, 2, 2))

container.sprites.add(Sprite("weapon_hammer_body", set_game, 2, 1, 4, 3))
container.sprites.add(Sprite("weapon_hammer_cursor", set_game, 0, 0, 2, 2))
container.sprites.add(Sprite("weapon_hammer_proj", set_game, 0, 0, 0, 0))

container.sprites.add(Sprite("weapon_ninja_body", set_game, 2, 10, 8, 2))
container.sprites.add(Sprite("weapon_ninja_cursor", set_game, 0, 10, 2, 2))
container.sprites.add(Sprite("weapon_ninja_proj", set_game, 0, 0, 0, 0))

container.sprites.add(Sprite("weapon_laser_body", set_game, 2, 12, 7, 3))
container.sprites.add(Sprite("weapon_laser_cursor", set_game, 0, 12, 2, 2))
container.sprites.add(Sprite("weapon_laser_proj", set_game, 10, 12, 2, 2))

container.sprites.add(Sprite("hook_chain", set_game, 2, 0, 1, 1))
container.sprites.add(Sprite("hook_head", set_game, 3, 0, 2, 1))

container.sprites.add(Sprite("weapon_ninja_muzzle1", set_game, 25, 0, 7, 4))
container.sprites.add(Sprite("weapon_ninja_muzzle2", set_game, 25, 4, 7, 4))
container.sprites.add(Sprite("weapon_ninja_muzzle3", set_game, 25, 8, 7, 4))

container.sprites.add(Sprite("pickup_health", set_game, 10, 2, 2, 2))
container.sprites.add(Sprite("pickup_armor", set_game, 12, 2, 2, 2))
container.sprites.add(Sprite("pickup_grenade", set_game, 2, 8, 7, 2))
container.sprites.add(Sprite("pickup_shotgun", set_game, 2, 6, 8, 2))
container.sprites.add(Sprite("pickup_laser", set_game, 2, 12, 7, 3))
container.sprites.add(Sprite("pickup_ninja", set_game, 2, 10, 8, 2))
container.sprites.add(Sprite("pickup_gun", set_game, 2, 4, 4, 2))
container.sprites.add(Sprite("pickup_hammer", set_game, 2, 1, 4, 3))

container.sprites.add(Sprite("flag_blue", set_game, 12, 8, 4, 8))
container.sprites.add(Sprite("flag_red", set_game, 16, 8, 4, 8))

container.sprites.add(Sprite("ninja_bar_full_left", set_game, 21, 4, 1, 2))
container.sprites.add(Sprite("ninja_bar_full", set_game, 22, 4, 1, 2))
container.sprites.add(Sprite("ninja_bar_empty", set_game, 23, 4, 1, 2))
container.sprites.add(Sprite("ninja_bar_empty_right", set_game, 24, 4, 1, 2))

container.sprites.add(Sprite("tee_body_outline", set_tee_body, 0, 0, 1, 1))
container.sprites.add(Sprite("tee_body", set_tee_body, 1, 0, 1, 1))
container.sprites.add(Sprite("tee_body_shadow", set_tee_body, 0, 1, 1, 1))
container.sprites.add(Sprite("tee_body_upper_outline", set_tee_body, 1, 1, 1, 1))

container.sprites.add(Sprite("tee_marking", set_tee_markings, 0, 0, 1, 1))

container.sprites.add(Sprite("tee_decoration", set_tee_decoration, 0, 0, 1, 1))
container.sprites.add(Sprite("tee_decoration_outline", set_tee_decoration, 1, 0, 1, 1))

container.sprites.add(Sprite("tee_hand", set_tee_hands, 0, 0, 1, 1))
container.sprites.add(Sprite("tee_hand_outline", set_tee_hands, 1, 0, 1, 1))

container.sprites.add(Sprite("tee_foot", set_tee_feet, 0, 0, 1, 1))
container.sprites.add(Sprite("tee_foot_outline", set_tee_feet, 1, 0, 1, 1))

container.sprites.add(Sprite("tee_eyes_normal", set_tee_eyes, 0, 0, 1, 1))
container.sprites.add(Sprite("tee_eyes_angry", set_tee_eyes, 1, 0, 1, 1))
container.sprites.add(Sprite("tee_eyes_pain", set_tee_eyes, 0, 1, 1, 1))
container.sprites.add(Sprite("tee_eyes_happy", set_tee_eyes, 1, 1, 1, 1))
container.sprites.add(Sprite("tee_eyes_surprise", set_tee_eyes, 0, 2, 1, 1))

container.sprites.add(Sprite("tee_hats_top1", set_tee_hats, 0, 0, 1, 1))
container.sprites.add(Sprite("tee_hats_top2", set_tee_hats, 0, 1, 1, 1))
container.sprites.add(Sprite("tee_hats_side1", set_tee_hats, 0, 2, 1, 1))
container.sprites.add(Sprite("tee_hats_side2", set_tee_hats, 0, 3, 1, 1))

container.sprites.add(Sprite("tee_bot_glow", set_tee_bot, 0, 0, 4, 4))
container.sprites.add(Sprite("tee_bot_foreground", set_tee_bot, 4, 0, 4, 4))
container.sprites.add(Sprite("tee_bot_background", set_tee_bot, 8, 0, 4, 4))

container.sprites.add(Sprite("oop", set_emoticons, 0, 0, 1, 1))
container.sprites.add(Sprite("exclamation", set_emoticons, 1, 0, 1, 1))
container.sprites.add(Sprite("hearts", set_emoticons, 2, 0, 1, 1))
container.sprites.add(Sprite("drop", set_emoticons, 3, 0, 1, 1))
container.sprites.add(Sprite("dotdot", set_emoticons, 0, 1, 1, 1))
container.sprites.add(Sprite("music", set_emoticons, 1, 1, 1, 1))
container.sprites.add(Sprite("sorry", set_emoticons, 2, 1, 1, 1))
container.sprites.add(Sprite("ghost", set_emoticons, 3, 1, 1, 1))
container.sprites.add(Sprite("sushi", set_emoticons, 0, 2, 1, 1))
container.sprites.add(Sprite("splattee", set_emoticons, 1, 2, 1, 1))
container.sprites.add(Sprite("deviltee", set_emoticons, 2, 2, 1, 1))
container.sprites.add(Sprite("zomg", set_emoticons, 3, 2, 1, 1))
container.sprites.add(Sprite("zzz", set_emoticons, 0, 3, 1, 1))
container.sprites.add(Sprite("wtf", set_emoticons, 1, 3, 1, 1))
container.sprites.add(Sprite("eyes", set_emoticons, 2, 3, 1, 1))
container.sprites.add(Sprite("question", set_emoticons, 3, 3, 1, 1))

container.sprites.add(Sprite("browse_lock_a", set_browseicons, 0, 0, 1, 1))
container.sprites.add(Sprite("browse_lock_b", set_browseicons, 0, 1, 1, 1))
container.sprites.add(Sprite("browse_unpure_a", set_browseicons, 1, 0, 1, 1))
container.sprites.add(Sprite("browse_unpure_b", set_browseicons, 1, 1, 1, 1))
container.sprites.add(Sprite("browse_star_a", set_browseicons, 2, 0, 1, 1))
container.sprites.add(Sprite("browse_star_b", set_browseicons, 2, 1, 1, 1))
container.sprites.add(Sprite("browse_heart_a", set_browseicons, 3, 0, 1, 1))
container.sprites.add(Sprite("browse_heart_b", set_browseicons, 3, 1, 1, 1))

container.sprites.add(Sprite("demobutton_play", set_demobuttons, 0, 0, 1, 1))
container.sprites.add(Sprite("demobutton_pause", set_demobuttons, 1, 0, 1, 1))
container.sprites.add(Sprite("demobutton_stop", set_demobuttons, 2, 0, 1, 1))
container.sprites.add(Sprite("demobutton_slower", set_demobuttons, 3, 0, 1, 1))
container.sprites.add(Sprite("demobutton_faster", set_demobuttons, 4, 0, 1, 1))

container.sprites.add(Sprite("file_demo1", set_fileicons, 0, 0, 1, 1))
container.sprites.add(Sprite("file_demo2", set_fileicons, 1, 0, 1, 1))
container.sprites.add(Sprite("file_folder", set_fileicons, 2, 0, 1, 1))
container.sprites.add(Sprite("file_map1", set_fileicons, 5, 0, 1, 1))
container.sprites.add(Sprite("file_map2", set_fileicons, 6, 0, 1, 1))

container.sprites.add(Sprite("guibutton_off", set_guibuttons, 0, 0, 4, 4))
container.sprites.add(Sprite("guibutton_on", set_guibuttons, 4, 0, 4, 4))
container.sprites.add(Sprite("guibutton_hover", set_guibuttons, 8, 0, 4, 4))

container.sprites.add(Sprite("guiicon_mute", set_guiicons, 0, 0, 4, 2))
container.sprites.add(Sprite("guiicon_friend", set_guiicons, 4, 0, 4, 2))

container.sprites.add(Sprite("menu_checkbox_active", set_menuicons, 0, 0, 1, 1))
container.sprites.add(Sprite("menu_checkbox_inactive", set_menuicons, 0, 1, 1, 1))
container.sprites.add(Sprite("menu_checkbox_hover", set_menuicons, 0, 2, 1, 1))
container.sprites.add(Sprite("menu_collapsed", set_menuicons, 1, 0, 1, 1))
container.sprites.add(Sprite("menu_expanded", set_menuicons, 1, 1, 1, 1))

container.sprites.add(Sprite("soundicon_on", set_soundicons, 0, 0, 1, 1))
container.sprites.add(Sprite("soundicon_mute", set_soundicons, 0, 1, 1, 1))

container.sprites.add(Sprite("tool_up_a", set_toolicons, 0, 0, 1, 1))
container.sprites.add(Sprite("tool_up_b", set_toolicons, 0, 1, 1, 1))
container.sprites.add(Sprite("tool_down_a", set_toolicons, 1, 0, 1, 1))
container.sprites.add(Sprite("tool_down_b", set_toolicons, 1, 1, 1, 1))
container.sprites.add(Sprite("tool_edit_a", set_toolicons, 2, 0, 1, 1))
container.sprites.add(Sprite("tool_edit_b", set_toolicons, 2, 1, 1, 1))
container.sprites.add(Sprite("tool_x_a", set_toolicons, 3, 0, 1, 1))
container.sprites.add(Sprite("tool_x_b", set_toolicons, 3, 1, 1, 1))

container.sprites.add(Sprite("arrow_left_a", set_arrowicons, 0, 0, 1, 1))
container.sprites.add(Sprite("arrow_left_b", set_arrowicons, 0, 1, 1, 1))
container.sprites.add(Sprite("arrow_left_c", set_arrowicons, 0, 2, 1, 1))
container.sprites.add(Sprite("arrow_up_a", set_arrowicons, 1, 0, 1, 1))
container.sprites.add(Sprite("arrow_up_b", set_arrowicons, 1, 1, 1, 1))
container.sprites.add(Sprite("arrow_up_c", set_arrowicons, 1, 2, 1, 1))
container.sprites.add(Sprite("arrow_right_a", set_arrowicons, 2, 0, 1, 1))
container.sprites.add(Sprite("arrow_right_b", set_arrowicons, 2, 1, 1, 1))
container.sprites.add(Sprite("arrow_right_c", set_arrowicons, 2, 2, 1, 1))
container.sprites.add(Sprite("arrow_down_a", set_arrowicons, 3, 0, 1, 1))
container.sprites.add(Sprite("arrow_down_b", set_arrowicons, 3, 1, 1, 1))
container.sprites.add(Sprite("arrow_down_c", set_arrowicons, 3, 2, 1, 1))

container.sprites.add(Sprite("friend_plus_a", set_friendicons, 0, 0, 1, 1))
container.sprites.add(Sprite("friend_plus_b", set_friendicons, 0, 1, 1, 1))
container.sprites.add(Sprite("friend_x_a", set_friendicons, 1, 0, 1, 1))
container.sprites.add(Sprite("friend_x_b", set_friendicons, 1, 1, 1, 1))

container.sprites.add(Sprite("level_a_on", set_levelicons, 0, 0, 1, 1))
container.sprites.add(Sprite("level_a_a", set_levelicons, 0, 1, 1, 1))
container.sprites.add(Sprite("level_a_b", set_levelicons, 0, 2, 1, 1))
container.sprites.add(Sprite("level_b_on", set_levelicons, 1, 0, 1, 1))
container.sprites.add(Sprite("level_b_a", set_levelicons, 1, 1, 1, 1))
container.sprites.add(Sprite("level_b_b", set_levelicons, 1, 2, 1, 1))
container.sprites.add(Sprite("level_c_on", set_levelicons, 2, 0, 1, 1))
container.sprites.add(Sprite("level_c_a", set_levelicons, 2, 1, 1, 1))
container.sprites.add(Sprite("level_c_b", set_levelicons, 2, 2, 1, 1))

container.sprites.add(Sprite("sidebar_refresh_a", set_sidebaricons, 0, 0, 1, 1))
container.sprites.add(Sprite("sidebar_refresh_b", set_sidebaricons, 0, 1, 1, 1))
container.sprites.add(Sprite("sidebar_friend_a", set_sidebaricons, 1, 0, 1, 1))
container.sprites.add(Sprite("sidebar_friend_b", set_sidebaricons, 1, 1, 1, 1))
container.sprites.add(Sprite("sidebar_filter_a", set_sidebaricons, 2, 0, 1, 1))
container.sprites.add(Sprite("sidebar_filter_b", set_sidebaricons, 2, 1, 1, 1))
container.sprites.add(Sprite("sidebar_info_a", set_sidebaricons, 3, 0, 1, 1))
container.sprites.add(Sprite("sidebar_info_b", set_sidebaricons, 3, 1, 1, 1))

container.sprites.add(Sprite("browser_a", set_browsericon, 0, 0, 1, 1))
container.sprites.add(Sprite("browser_b", set_browsericon, 0, 1, 1, 1))

container.sprites.add(Sprite("timerclock_a", set_timerclock, 0, 0, 1, 1))
container.sprites.add(Sprite("timerclock_b", set_timerclock, 0, 1, 1, 1))

anim = Animation("base")
anim.body.frames.add(AnimKeyframe(0, 0, -4, 0))
anim.back_foot.frames.add(AnimKeyframe(0, 0, 10, 0))
anim.front_foot.frames.add(AnimKeyframe(0, 0, 10, 0))
container.animations.add(anim)

anim = Animation("idle")
anim.back_foot.frames.add(AnimKeyframe(0, -7, 0, 0))
anim.front_foot.frames.add(AnimKeyframe(0, 7, 0, 0))
container.animations.add(anim)

anim = Animation("inair")
anim.back_foot.frames.add(AnimKeyframe(0, -3, 0, -0.1))
anim.front_foot.frames.add(AnimKeyframe(0, 3, 0, -0.1))
container.animations.add(anim)

anim = Animation("walk")
anim.body.frames.add(AnimKeyframe(0.0, 0, 0, 0))
anim.body.frames.add(AnimKeyframe(0.2, 0, -1, 0))
anim.body.frames.add(AnimKeyframe(0.4, 0, 0, 0))
anim.body.frames.add(AnimKeyframe(0.6, 0, 0, 0))
anim.body.frames.add(AnimKeyframe(0.8, 0, -1, 0))
anim.body.frames.add(AnimKeyframe(1.0, 0, 0, 0))

anim.back_foot.frames.add(AnimKeyframe(0.0, 8, 0, 0))
anim.back_foot.frames.add(AnimKeyframe(0.2, -8, 0, 0))
anim.back_foot.frames.add(AnimKeyframe(0.4, -10, -4, 0.2))
anim.back_foot.frames.add(AnimKeyframe(0.6, -8, -8, 0.3))
anim.back_foot.frames.add(AnimKeyframe(0.8, 4, -4, -0.2))
anim.back_foot.frames.add(AnimKeyframe(1.0, 8, 0, 0))

anim.front_foot.frames.add(AnimKeyframe(0.0, -10, -4, 0.2))
anim.front_foot.frames.add(AnimKeyframe(0.2, -8, -8, 0.3))
anim.front_foot.frames.add(AnimKeyframe(0.4, 4, -4, -0.2))
anim.front_foot.frames.add(AnimKeyframe(0.6, 8, 0, 0))
anim.front_foot.frames.add(AnimKeyframe(0.8, 8, 0, 0))
anim.front_foot.frames.add(AnimKeyframe(1.0, -10, -4, 0.2))
container.animations.add(anim)

anim = Animation("hammer_swing")
anim.attach.frames.add(AnimKeyframe(0.0, 0, 0, -0.10))
anim.attach.frames.add(AnimKeyframe(0.3, 0, 0, 0.25))
anim.attach.frames.add(AnimKeyframe(0.4, 0, 0, 0.30))
anim.attach.frames.add(AnimKeyframe(0.5, 0, 0, 0.25))
anim.attach.frames.add(AnimKeyframe(1.0, 0, 0, -0.10))
container.animations.add(anim)

anim = Animation("ninja_swing")
anim.attach.frames.add(AnimKeyframe(0.00, 0, 0, -0.25))
anim.attach.frames.add(AnimKeyframe(0.10, 0, 0, -0.05))
anim.attach.frames.add(AnimKeyframe(0.15, 0, 0, 0.35))
anim.attach.frames.add(AnimKeyframe(0.42, 0, 0, 0.40))
anim.attach.frames.add(AnimKeyframe(0.50, 0, 0, 0.35))
anim.attach.frames.add(AnimKeyframe(1.00, 0, 0, -0.25))
container.animations.add(anim)

weapon = WeaponSpec(container, "hammer")
weapon.firedelay.set_value(125)
weapon.damage.set_value(3)
weapon.visual_size.set_value(96)
weapon.offsetx.set_value(4)
weapon.offsety.set_value(-20)
container.weapons.hammer.base.set_value(weapon)
container.weapons.id.add(weapon)

weapon = WeaponSpec(container, "gun")
weapon.firedelay.set_value(125)
weapon.damage.set_value(1)
weapon.ammoregentime.set_value(500)
weapon.visual_size.set_value(64)
weapon.offsetx.set_value(32)
weapon.offsety.set_value(-4)
weapon.muzzleoffsetx.set_value(50 + 8.8752)  # see gun in 0.6 content.py for the number after the plus sign (TODO: also copy the comment from 0.6 content.py, if it gets removed)
weapon.muzzleoffsety.set_value(6)
container.weapons.gun.base.set_value(weapon)
container.weapons.id.add(weapon)

weapon = WeaponSpec(container, "shotgun")
weapon.firedelay.set_value(500)
weapon.damage.set_value(1)
weapon.visual_size.set_value(96)
weapon.offsetx.set_value(24)
weapon.offsety.set_value(-2)
weapon.muzzleoffsetx.set_value(70 + 13.3128)  # see gun in 0.6 content.py for the number after the plus sign (TODO: also copy the comment from 0.6 content.py, if it gets removed)
weapon.muzzleoffsety.set_value(6)
container.weapons.shotgun.base.set_value(weapon)
container.weapons.id.add(weapon)

weapon = WeaponSpec(container, "grenade")
weapon.firedelay.set_value(500)  # TODO: fix this
weapon.damage.set_value(6)
weapon.visual_size.set_value(96)
weapon.offsetx.set_value(24)
weapon.offsety.set_value(-2)
container.weapons.grenade.base.set_value(weapon)
container.weapons.id.add(weapon)

weapon = WeaponSpec(container, "laser")
weapon.firedelay.set_value(800)
weapon.damage.set_value(5)
weapon.visual_size.set_value(92)
weapon.offsetx.set_value(24)
weapon.offsety.set_value(-2)
container.weapons.laser.base.set_value(weapon)
container.weapons.id.add(weapon)

weapon = WeaponSpec(container, "ninja")
weapon.firedelay.set_value(800)
weapon.damage.set_value(9)
weapon.visual_size.set_value(96)
weapon.offsetx.set_value(0)
weapon.offsety.set_value(0)
weapon.muzzleoffsetx.set_value(40)
weapon.muzzleoffsety.set_value(-4)
container.weapons.ninja.base.set_value(weapon)
container.weapons.id.add(weapon)
