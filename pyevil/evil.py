from . import _evil
import ctypes
from collections.abc import Collection

def id2obj(addr):
    return _evil.id2obj(addr)

def addrof(obj):
    return _evil.addrof(obj)

def rawdump(obj):
    return _evil.rawdump()

def rawload(mem):
    return _evil.rawload(mem)

def getrefcount(obj):
    return _evil.getrefcount(obj)

def setrefcount(obj, refcnt):
    if refcnt<0:
        raise ValueError("new refcount must be greater than 0")
    _evil.setrefcount(obj, refcnt)

def mk_immortal(obj):
    _evil.mk_immortal(obj)

def eternize(obj):
    """Makes the argument and all of its items immortal"""
    if isinstance(obj, Collection):
        for item in obj:
            eternize(item)
    mk_immortal(obj)

def settype(obj, tp):
    _evil.settype(obj, tp)

def getsize(obj):
    return _evil.getsize(obj)

def setsize(obj, sz):
    return _evil.setsize(obj, sz)

def forceset(tgt, obj, immortalize=False):
    _evil.forceset(tgt, obj)
    if immortalize and _evil.has_immortal():
        mk_immortal(tgt)

def settupleitem(tup, index, obj):
    _evil.settupleitem(tup, index, obj)
