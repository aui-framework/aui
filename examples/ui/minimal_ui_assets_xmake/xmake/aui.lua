import("lib.detect.find_program")
import("core.project.config")

function _get_gen_path()
  return path.join(path.absolute(config.builddir()), ".gens", "aui")
end

function _get_aui_package(target)
  -- Get the aui package from target
  for _, pkg in pairs(target:pkgs()) do
    if pkg:name() == "aui" then
      return pkg
    end
  end
  return nil
end

function _pack_asset(assets_dir, asset_path, aui_installdir)
  local toolbox_path = path.join(aui_installdir, "bin", "aui.toolbox")

  if not os.isfile(toolbox_path) then
    toolbox_path = path.join(aui_installdir, "bin", "aui.toolbox.exe")
  end

  if not os.isfile(toolbox_path) then
    toolbox_path = find_program("aui.toolbox")
  end
  
  wprint("Using toolbox: " .. toolbox_path)
  os.execv(toolbox_path, {
    "pack",
    assets_dir,
    asset_path,
    path.join(_get_gen_path(), hash.sha256(asset_path) .. ".cpp")
  })
end

function assets(target)
  -- Get aui package install directory
  local aui_pkg = _get_aui_package(target)
  if not aui_pkg then
    raise("aui package not found in target")
  end
  
  local aui_installdir = aui_pkg:installdir()
  wprint("AUI package install directory: " .. aui_installdir)
  local assets_dir = path.join(path.absolute(target:scriptdir()), "assets")
  for _, file in ipairs(os.files(path.join(assets_dir, "**"))) do
    _pack_asset(assets_dir, file, aui_installdir)
  end

  -- Add generated files to the target
  target:add("files", path.join(_get_gen_path(), "**.cpp"))
end
