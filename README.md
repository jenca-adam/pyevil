# `pyevil`

A python module that groups together a bunch of voodoo c black magic function that can be useful when dealing with stuff at the very lowest of low levels of Python programming.

## A disclaimer

Almost every function in this module is **dangerous** and can cause **SEGFAULTS** or **MEMORY LEAKS**.
**NEVER** use this in production.

## Functions

### `id2obj()`

Converts an address to a Python object.

Note that despite the misleading name, this function is **not guaranteed** to restore the object if it was garbage collected.
Also this relies on the implementation of `id()` - (i.e. it will only do what's advertised if you're using CPython)
### `addrof()`

Converts a Python object to an address.

Coincidentally, this does the same thing as `id()` in CPython, but it actually returns the address of the object regardless of the implementation of `id()`.

### `rawdump()`

Gets the raw memory representation of a Python object.

Can be used together with `rawload()` to create a super-rudimentary **unsafe**, **never-use-this** version of `pickle`.
If the object itself or any object it points to was GC'd or had their address changed, `rawload()` will fail to return a well-formed object.
As a result, you **won't** get the same result when `rawload()` is called in another Python instance.

### `rawload()`

Creates a Python object from its raw memory representation

See the note about `rawdump()`

### `getrefcount()`

Returns the reference count of a Python object.

Does the same as `sys.getrefcount()`

### `setrefcount()`

Sets the reference count of a Python object.

Using this can cause memory leaks if you increase the reference count or segmentation faults if you decrease it.
Use with care.

### `mk_immortal()`

Sets the reference count of a Python object to `_Py_IMMORTAL_REFCNT`

### `eternize()`

Makes the object and all its items (if it's a container) immortal.

### `settype()`

Sets the `ob_type` of an object without any conversion being done whatsoever.

Using this will likely create a malformed object.

### `getsize()`

Gets the `ob_size` of an object.

Returns bogus values on objects that don't have sizes.

### `setsize()`

Sets the `ob_size` of an object.

Decreasing will likely cause memory leaks, increasing will likely cause segfaults.

Setting the size of an object that doesn't have a size will create a malformed object.
### `forceset()`

Directly sets the value of an object to the value of another object.
To avoid data being overwritten, both objects must have the same size.

Can cause serious refcount issues, so I'd advise you to call `mk_immortal()` on the target after each use. (setting the kwarg `immortalize` to `True` will do this for you).
I know this is not how you solve problems but I am too lazy to fix the reference counting. 
**CAN ALSO CRASH THE PYTHON SHELL!**
