import argparse
import ctypes
from enum import Enum, auto
import os


class CommonPkgHdr(ctypes.Structure):
    _fields_ = [
        ('id', ctypes.c_int),
        ('content_size', ctypes.c_int),
    ]

class CommonPkg():
    def __init__(self):
        self.id = None
        self.content_size = None
        self.content = None

class PkgTime(ctypes.Structure):
    _fields_ = [
        ('time', ctypes.c_double),
    ]

class PkgObjId(ctypes.Structure):
    _fields_ = [
        ('id', ctypes.c_uint),
    ]

class PkgSpeed(ctypes.Structure):
    _fields_ = [
        ('speed', ctypes.c_double),
    ]

class PkgPositions(ctypes.Structure):
    _fields_ = [
        ('x', ctypes.c_double),
        ('y', ctypes.c_double),
        ('z', ctypes.c_double),
        ('h', ctypes.c_double),
        ('r', ctypes.c_double),
        ('p', ctypes.c_double),
    ]

class PkgId(Enum):
    HEADER      = 11
    TIME_SERIES = 12
    OBJ_ID      = 13
    MODEL_ID    = 14
    POSITIONS   = 15
    SPEED       = 16
    OBJ_TYPE    = 17
    OBJ_CATEGORY = 18
    CTRL_TYPE   = 19
    WHEEL_ANGLE = 20
    WHEEL_ROT   = 21
    BOUNDING_BOX = 22
    SCALE_MODE = 23
    VISIBILITY_MASK = 24
    NAME = 25
    ROAD_ID = 26
    LANE_ID = 27
    POS_OFFSET = 28
    POS_T = 29
    POS_S = 30
    OBJ_DELETED = 31
    OBJ_ADDED = 32
    END_OF_SCENARIO = 33

class objectState():
    def __init__(self):
        self.obj_id = None
        self.obj_active = None
        self.speed = None
        self.pos = None
        self.name = None

class CompleteObjectState():
    def __init__(self):
        self.time = None
        self.objectState_ = []

file_read = open('simple_scenario.dat', 'rb')
stat = os.stat(file_read.name)
pkgs = []
while True:
    if file_read.tell() == stat.st_size:
        break # reach end of file
    header_buffer = file_read.read(ctypes.sizeof(CommonPkgHdr))

    header = CommonPkgHdr.from_buffer_copy(header_buffer)
    pkg = CommonPkg()
    pkg.id = header.id
    pkg.content_size = header.content_size

    if header.id == PkgId.HEADER.value:
        version_buffer = file_read.read(ctypes.sizeof(ctypes.c_int))
        version = ctypes.c_int.from_buffer_copy(version_buffer)
        odr_size_buffer = file_read.read(ctypes.sizeof(ctypes.c_int))
        odr_size = ctypes.c_int.from_buffer_copy(odr_size_buffer)
        odr_filename_bytes = file_read.read(odr_size.value)
        odr_filename = odr_filename_bytes[:odr_size.value].decode('utf-8')

        mdl_size_buffer = file_read.read(ctypes.sizeof(ctypes.c_int))
        mdl_size = ctypes.c_int.from_buffer_copy(mdl_size_buffer)
        mdl_filename_bytes = file_read.read(mdl_size.value)
        mdl_filename = mdl_filename_bytes[:mdl_size.value].decode('utf-8')

    elif header.id == PkgId.TIME_SERIES.value:
        time_buffer = file_read.read(header.content_size)
        t = PkgTime.from_buffer_copy(time_buffer)
        pkg.content = t
        pkgs.append(pkg)
    elif header.id == PkgId.OBJ_ID.value:
        obj_id_buffer = file_read.read(header.content_size)
        obj_id = PkgObjId.from_buffer_copy(obj_id_buffer)
        pkg.content = obj_id
        pkgs.append(pkg)
    elif header.id == PkgId.OBJ_ADDED.value:
        print (" obj added ")
        pkgs.append(pkg)
    elif header.id == PkgId.SPEED.value:
        speed_buffer = file_read.read(header.content_size)
        speed = PkgSpeed.from_buffer_copy(speed_buffer)
        pkg.content = speed
        pkgs.append(pkg)
    elif header.id == PkgId.POSITIONS.value:
        pos_buffer = file_read.read(header.content_size)
        pos = PkgPositions.from_buffer_copy(pos_buffer)
        pkg.content = pos
        pkgs.append(pkg)
    elif header.id == PkgId.NAME.value:
        name_buffer = file_read.read(header.content_size)
        name = name_buffer[:header.content_size].decode('utf-8')
        pkg.content = name
        pkgs.append(pkg)
file_read.close()

first_time_frame = False
new_obj = False
CompleteObjectState_ = CompleteObjectState()
objectState_ = objectState()
for pkg in pkgs:
    if pkg.id == PkgId.TIME_SERIES.value:
        if first_time_frame == True:
            break
        CompleteObjectState_.time = pkg.content
        first_time_frame = True
    elif pkg.id == PkgId.OBJ_ID.value:
        if new_obj == True:
            CompleteObjectState_.objectState_ .append(objectState_) # append for each object
            objectState_ = objectState()
            new_obj = False
        objectState_.obj_id = pkg.content
        new_obj = True
    elif pkg.id == PkgId.OBJ_ADDED.value:
        objectState_.obj_active = True
    elif pkg.id == PkgId.SPEED.value:
        objectState_.speed = pkg.content
    elif pkg.id == PkgId.POSITIONS.value:
        objectState_.pos = pkg.content
    if pkg.id == PkgId.NAME.value:
        objectState_.name = pkg.content
CompleteObjectState_.objectState_ .append(objectState_)

# change the Value
CompleteObjectState_.objectState_[0].speed = 100
print (" done ")









