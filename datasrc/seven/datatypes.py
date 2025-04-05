def only(x):
	if len(x) != 1:
		raise ValueError
	return list(x)[0]


global_id_counter = 0


def get_id():
	global global_id_counter
	global_id_counter += 1
	return global_id_counter


def get_uid():
	return f"x{int(get_id())}"


def fix_casing(Str):
	NewStr = ""
	NextUpperCase = True
	for c in Str:
		if NextUpperCase:
			NextUpperCase = False
			NewStr += c.upper()
		else:
			if c == "_":
				NextUpperCase = True
			else:
				NewStr += c.lower()
	return NewStr


def format_name(typ, name):
	if "*" in typ:
		return "m_p" + fix_casing(name)
	if "[]" in typ:
		return "m_a" + fix_casing(name)
	return "m_" + fix_casing(name)


class BaseType:
	def __init__(self, type_name):
		self._type_name = type_name
		self._target_name = "INVALID"
		self._id = get_id()  # this is used to remember what order the members have in structures etc

	def identifier(self):
		return "x" + str(self._id)

	def target_name(self):
		return self._target_name

	def type_name(self):
		return self._type_name

	def id(self):
		return self._id

	def emit_declaration(self, name):
		return [f"{self.type_name()} {format_name(self.type_name(), name)};"]

	def emit_pre_definition(self, target_name):
		self._target_name = target_name
		return []

	def emit_definition(self, _name):
		return []


class MemberType:
	def __init__(self, name, var):
		self.name = name
		self.var = var


class Struct(BaseType):
	def __init__(self, type_name):
		BaseType.__init__(self, type_name)

	def members(self):
		m = []
		for name, value in self.__dict__.items():
			if name[0] == "_":
				continue
			m += [MemberType(name, value)]
		m.sort(key=lambda i: i.var.id)
		return m

	def emit_type_declaration(self, _name):
		lines = []
		lines += ["struct " + self.type_name()]
		lines += ["{"]
		for member in self.members():
			lines += ["\t" + line for line in member.var.EmitDeclaration(member.name)]
		lines += ["};"]
		return lines

	def emit_pre_definition(self, target_name):
		BaseType.emit_pre_definition(self, target_name)
		lines = []
		for member in self.members():
			lines += member.var.EmitPreDefinition(target_name + "." + member.name)
		return lines

	def emit_definition(self, _name):
		lines = [f"/* {self.target_name()} */ {{"]
		for member in self.members():
			lines += ["\t" + " ".join(member.var.emit_definition("")) + ","]
		lines += ["}"]
		return lines


class Array(BaseType):
	def __init__(self, typ):
		BaseType.__init__(self, typ.TypeName())
		self.type = typ
		self.items = []

	def add(self, instance):
		if instance.TypeName() != self.type.TypeName():
			raise ValueError()
		self.items += [instance]

	def emit_declaration(self, name):
	return [f"int m_Num{fix_casing(name)};",
         f"{self.type_name()} *{format_name('[]', name)};"]

	def EmitPreDefinition(self, target_name):
		BaseType.emit_pre_definition(self, target_name)

		lines = []
		i = 0
		for item in self.items:
			lines += item.EmitPreDefinition(f"{self.identifier()}[{int(i)}]")
			i += 1

		if self.items:
			lines += [f"static {self.type_name()} {self.identifier()}[] = {{"]
			for item in self.items:
				itemlines = item.emit_definition("")
				lines += ["\t" + " ".join(itemlines).replace("\t", " ") + ","]
			lines += ["};"]
		else:
			lines += [f"static {self.type_name()} *{self.identifier()} = nullptr;"]

		return lines

	def emit_definition(self, _name):
		return [str(len(self.items)) + "," + self.identifier()]


# Basic Types


class Int(BaseType):
	def __init__(self, value):
		BaseType.__init__(self, "int")
		self.value = value

	def set_value(self, value):
		self.value = value

	def emit_definition(self, _name):
		return [f"{int(self.value)}"]


class Float(BaseType):
	def __init__(self, value):
		BaseType.__init__(self, "float")
		self.value = value

	def set_value(self, value):
		self.value = value

	def emit_definition(self, _name):
		return [f"{self.value:f}f"]


class String(BaseType):
	def __init__(self, value):
		BaseType.__init__(self, "const char*")
		self.value = value

	def set_value(self, value):
		self.value = value

	def emit_definition(self, _name):
		return ["\"" + self.value + "\""]


class Pointer(BaseType):
	def __init__(self, typ, target):
		BaseType.__init__(self, f"{typ().TypeName()}*")
		self.target = target

	def set_value(self, target):
		self.target = target

	def emit_definition(self, _name):
		return ["&" + self.target.TargetName()]


class TextureHandle(BaseType):
	def __init__(self):
		BaseType.__init__(self, "IGraphics::CTextureHandle")

	def emit_definition(self, _name):
		return ["IGraphics::CTextureHandle()"]


class SampleHandle(BaseType):
	def __init__(self):
		BaseType.__init__(self, "ISound::CSampleHandle")

	def emit_definition(self, _name):
		return ["ISound::CSampleHandle()"]


# Helper functions


def emit_type_declaration(root):
	for line in root().emit_type_declaration(""):
		print(line)


def emit_definition(root, name):
	for line in root.EmitPreDefinition(name):
		print(line)
	print(f"{root.TypeName()} {name} = ")
	for line in root.emit_definition(name):
		print(line)
	print(";")


# Network stuff


class Object:
	pass


class Enum:
	def __init__(self, name, values):
		self.name = name
		self.values = values


class Flags:
	def __init__(self, name, values):
		self.name = name
		self.values = values


class NetObject:
	def __init__(self, name, variables):
		line = name.split(":")
		self.name = line[0]
		self.base = None
		self.base_struct_name = None
		if len(line) > 1:
			self.base = line[1]
			self.base_struct_name = f"CNetObj_{self.base}"
		self.struct_name = f"CNetObj_{self.name}"
		self.enum_name = f"NETOBJTYPE_{self.name.upper()}"
		self.variables = variables

	def emit_declaration(self):
		if self.base is not None:
			lines = [f"struct {self.struct_name} : public {self.base_struct_name}", "{"]
		else:
			lines = [f"struct {self.struct_name}", "{"]
		lines += ["\tusing is_sixup = char;"]
		lines += [f"\tstatic constexpr int ms_MsgId = {self.enum_name};"]
		for v in self.variables:
			lines += ["\t" + line for line in v.emit_declaration()]
		lines += ["};"]
		return lines

	def emit_validate(self, objects):
		lines = [f"case {self.enum_name}:"]
		lines += ["{"]
		lines += [f"\t{self.struct_name} *pObj = ({self.struct_name} *)pData;"]
		lines += ["\tif(sizeof(*pObj) != Size) return -1;"]

		variables = self.variables
		next_base_name = self.base
		while next_base_name is not None:
			base_item = only([i for i in objects if i.name == next_base_name])
			variables = base_item.variables + variables
			next_base_name = base_item.base

		for v in variables:
			lines += ["\t" + line for line in v.emit_validate()]
		lines += ["\treturn 0;"]
		lines += ["}"]
		return lines


class NetEvent(NetObject):
	def __init__(self, name, variables):
		NetObject.__init__(self, name, variables)
		if self.base is not None:
			self.base_struct_name = f"CNetEvent_{self.base}"
		self.struct_name = f"CNetEvent_{self.name}"
		self.enum_name = f"NETEVENTTYPE_{self.name.upper()}"


class NetMessage(NetObject):
	def __init__(self, name, variables):
		NetObject.__init__(self, name, variables)
		if self.base is not None:
			self.base_struct_name = f"CNetMsg_{self.base}"
		self.struct_name = f"CNetMsg_{self.name}"
		self.enum_name = f"NETMSGTYPE_{self.name.upper()}"

	def emit_unpack(self):
		lines = []
		lines += [f"case {self.enum_name}:"]
		lines += ["{"]
		lines += [f"\t{self.struct_name} *pMsg = ({self.struct_name} *)m_aMsgData;"]
		lines += ["\t(void)pMsg;"]
		for v in self.variables:
			lines += ["\t" + line for line in v.emit_unpack()]
		for v in self.variables:
			lines += ["\t" + line for line in v.emit_unpack_check()]
		lines += ["} break;"]
		return lines

	def emit_declaration(self):
		extra = []
		extra += ["\t"]
		extra += ["\tbool Pack(CMsgPacker *pPacker) const"]
		extra += ["\t{"]
		for v in self.variables:
			extra += ["\t\t" + line for line in v.emit_pack()]
		extra += ["\t\treturn pPacker->Error() != 0;"]
		extra += ["\t}"]

		lines = NetObject.emit_declaration(self)
		lines = lines[:-1] + extra + lines[-1:]
		return lines


class NetVariable:
	def __init__(self, name, default=None):
		self.name = name
		self.default = None if default is None else str(default)

	def emit_declaration(self):
		return []

	def emit_validate(self):
		return []

	def emit_pack(self):
		return []

	def emit_unpack(self):
		return []

	def emit_unpack_check(self):
		return []


class NetString(NetVariable):
	def emit_declaration(self):
		return [f"const char *{self.name};"]

	def emit_unpack(self):
		return [f"pMsg->{self.name} = pUnpacker->GetString();"]

	def emit_pack(self):
		return [f"pPacker->AddString({self.name}, -1);"]


class NetStringStrict(NetVariable):
	def emit_declaration(self):
		return [f"const char *{self.name};"]

	def emit_unpack(self):
		return [f"pMsg->{self.name} = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);"]

	def emit_pack(self):
		return [f"pPacker->AddString({self.name}, -1);"]


class NetIntAny(NetVariable):
	def emit_declaration(self):
		return [f"int {self.name};"]

	def emit_unpack(self):
		if self.default is None:
			return [f"pMsg->{self.name} = pUnpacker->GetInt();"]
		return [f"pMsg->{self.name} = pUnpacker->GetIntOrDefault({self.default});"]

	def emit_pack(self):
		return [f"pPacker->AddInt({self.name});"]


class NetIntRange(NetIntAny):
	def __init__(self, name, min_val, max_val, default=None):
		NetIntAny.__init__(self, name, default=default)
		self.min = str(min_val)
		self.max = str(max_val)

	def emit_validate(self):
		return [f"if(!CheckInt(\"{self.name}\", pObj->{self.name}, {self.min}, {self.max})) return -1;"]

	def emit_unpack_check(self):
		return [f"if(!CheckInt(\"{self.name}\", pMsg->{self.name}, {self.min}, {self.max})) break;"]


class NetEnum(NetIntRange):
	def __init__(self, name, enum):
		NetIntRange.__init__(self, name, 0, len(enum.values) - 1)


class NetFlag(NetIntAny):
	def __init__(self, name, flag):
		NetIntAny.__init__(self, name)
		if len(flag.values) > 0:
			self.mask = f"{flag.name}_{flag.values[0]}"
			for i in flag.values[1:]:
				self.mask += f"|{flag.name}_{i}"
		else:
			self.mask = "0"

	def emit_validate(self):
		return [f"if(!CheckFlag(\"{self.name}\", pObj->{self.name}, {self.mask})) return -1;"]

	def emit_unpack_check(self):
		return [f"if(!CheckFlag(\"{self.name}\", pMsg->{self.name}, {self.mask})) break;"]


class NetBool(NetIntRange):
	def __init__(self, name, default=None):
		default = None if default is None else int(default)
		NetIntRange.__init__(self, name, 0, 1, default=default)


class NetTick(NetIntRange):
	def __init__(self, name):
		NetIntRange.__init__(self, name, 0, "max_int")


class NetArray(NetVariable):
	def __init__(self, var, size):
		NetVariable.__init__(self, var.name)
		self.base_name = var.name
		self.var = var
		self.size = size
		self.name = self.base_name + f"[{int(self.size)}]"

	def emit_declaration(self):
		self.var.name = self.name
		return self.var.emit_declaration()

	def emit_validate(self):
		lines = []
		for i in range(self.size):
			self.var.name = self.base_name + f"[{int(i)}]"
			lines += self.var.emit_validate()
		return lines

	def emit_unpack(self):
		lines = []
		for i in range(self.size):
			self.var.name = self.base_name + f"[{int(i)}]"
			lines += self.var.emit_unpack()
		return lines

	def emit_pack(self):
		lines = []
		for i in range(self.size):
			self.var.name = self.base_name + f"[{int(i)}]"
			lines += self.var.emit_pack()
		return lines

	def emit_unpack_check(self):
		lines = []
		for i in range(self.size):
			self.var.name = self.base_name + f"[{int(i)}]"
			lines += self.var.emit_unpack_check()
		return lines
