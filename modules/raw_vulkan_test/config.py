def can_build(env, platform):
    # Example renderer: raw Vulkan on X11 only.
    return platform == "linuxbsd" and env["vulkan"] and env["x11"]


def configure(env):
    pass


def is_enabled():
    # Not built by default. Enable with `module_raw_vulkan_test_enabled=yes`.
    return False
