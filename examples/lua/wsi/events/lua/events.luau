local function main()
  local window = wsi.window.open("test", 800, 600, wsi.window_flag.RESIZEABLE)

  window:on_closed(function()
    print("Close event!")
    return true
  end)

  window:on_resized(function(extent)
    print("Resize event:\n   {}", extent)
  end)

  window:on_monitor_changed(function(monitor)
    print("Monitor changed event:\n   {}", monitor)
  end)

  window:on_mouse_moved(function(_, position)
    print("Mouse move event:\n   {}", position)
  end)

  window:on_mouse_button_down(function(_, button, position)
    print("Mouse button down event:\n   {}\n   {}", button, position)
  end)

  window:on_mouse_button_up(function(_, button, position)
    print("Mouse button up event:\n   {}\n   {}", button, position)
  end)

  window:on_restored(function()
    print("Restored event!")
  end)

  window:on_minimized(function()
    print("Minimzed event!")
  end)

  window:on_activated(function()
    print("Activate event!")
  end)

  window:on_deactivated(function()
    print("Deactivate event!")
  end)

  local foo = 0
  window:on_key_down(function(_, key, c)
    print("Key down event: {} {}", key, c)

    local closures = {
      [wsi.key.ESCAPE] = function()
        print("Closing window!")
        window:close()
      end,
      [wsi.key.W] = function()
        local extent = window:extent()
        extent.width = extent.width + 10
        window:set_extent(extent)
      end,
      [wsi.key.T] = function()
        foo = foo + 1
        window:set_title(format("StormKit Lua Events Example | T pressed {} times", foo))
      end,
      [wsi.key.H] = function()
        local extent = window:extent()
        extent.height = extent.height + 10
        window:set_extent(extent)
      end,
      [wsi.key.F11] = function()
        window:toggle_fullscreen()
        print("Toggling fullscreen to {}", window:fullscreen())
      end,
      [wsi.key.F1] = function()
        window:toggle_hidden_mouse()
        print("Toggling hidden mouse to {}", window:is_mouse_hidden())
      end,
      [wsi.key.F2] = function()
        window:toggle_locked_mouse()
        print("Toggling locked mouse to {}", window:is_mouse_locked())
      end,
      [wsi.key.F3] = function()
        window:toggle_confined_mouse()
        print("Toggling confined mouse to {}", window:is_mouse_confined())
      end,
      [wsi.key.F4] = function()
        window:toggle_relative_mouse()
        print("Toggling relative mouse to {}", window:is_mouse_relative())
      end,
      [wsi.key.F5] = function()
        window:toggle_key_repeat()
        print("Toggling key repeat to {}", window:is_key_repeat_enabled())
      end,
    }

    for k, closure in pairs(closures) do
      if k == key then
        closure()
        break
      end
    end
  end)

  window:on_key_up(function(_, key, c)
    print("Key up event: {} {}", key, c)
  end)

  window:event_loop(function()
    window:clear()
  end)
end

main()
