import AppKit
import SwiftStormKitCpp
import SwiftStormKitObjc

final class Window_: NSWindow {
    override var acceptsFirstResponder: Bool { return true }
    override var canBecomeKey: Bool { return true }
    override var canBecomeMain: Bool { return true }
}

extension NSWindow {
    var titlebarHeight: CGFloat {
        frame.height - contentRect(forFrameRect: frame).height
    }
}

public typealias WindowID = CUnsignedLongLong
final class WindowController: NSWindowController, NSWindowDelegate {
    var view: View
    var id: WindowID
    let metalLayer: Bool
    init(
        title: String,
        width: CDouble,
        height: CDouble,
        resizeable: CBool,
        borderless: CBool,
        metalLayer: CBool,
        id: WindowID
    ) {
        self.id = id
        self.metalLayer = metalLayer

        var mask: NSWindow.StyleMask = [.closable, .miniaturizable, .fullSizeContentView]
        if resizeable {
            mask = mask.union([.resizable])
        }
        if !borderless {
            mask = mask.union([.titled])
        }

        let window_ =
            Window_(
                contentRect: NSRect(x: 0, y: 0, width: width, height: height),
                styleMask: [.closable, .titled, .resizable, .miniaturizable],
                backing: .buffered,
                defer: true
            )

        if let contentView = window_.contentView {
            let frame = window_.convertToBacking(contentView.frame)

            view = View(frame: frame, id: self.id)
            view.wantsLayer = false
            if metalLayer {
                view.wantsLayer = true
                view.layer = CAMetalLayer()
            }

            super.init(window: window_)

            window_.delegate = self
            window_.contentView = view
            window_.isOpaque = true
            window_.title = title

            window_.makeFirstResponder(self)
            window_.center()
            window_.makeKeyAndOrderFront(self)
        } else {
            fatalError("Swift: init(title: String, width: CDouble, height: CDouble, id: WindowID) failed to open window")
        }
    }

    required init?(coder aCoder: NSCoder) {
        fatalError("Swift: init(coder:) not implemented!")
    }

    func drawBitmap(data: UnsafeMutablePointer<CUnsignedChar>) {
        if let window {
            view.drawBitmap(data: data, frame: window.frame, offset_y: window.titlebarHeight)
        }
    }

    /* NSWindowDelegate */
    func windowDidChangeScreen(_ notification: Notification) {

    }

    func windowDidResize(_ notification: Notification) {
        if let window {
            swiftResizedEvent(id, window.frame.width.cf, window.frame.height.cf - window.titlebarHeight.cf)
        }
    }

    func windowDidDeminiaturize(_ notification: Notification) {
        swiftRestoredEvent(id)
    }

    func windowDidMiniaturize(_ notification: Notification) {
        swiftMinimizedEvent(id)
    }

    func windowDidResignKey(_ notification: Notification) {
        swiftDeactivatedEvent(id)
    }

    func windowDidBecomeKey(_ notification: Notification) {
        swiftActivatedEvent(id)
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        true
    }

    func windowShouldClose(_ sender: NSWindow) -> Bool {
        return swiftClosedEvent(id)
    }
}

public final class Window {
    let controller: WindowController
    public init(
        title: String,
        width: CDouble,
        height: CDouble,
        resizeable: CBool,
        borderless: CBool,
        metalLayer: CBool,
        id: WindowID
    ) {
        controller = WindowController(
            title: title,
            width: width,
            height: height,
            resizeable: resizeable,
            borderless: borderless,
            metalLayer: metalLayer,
            id: id
        )
    }

    public func updateID(id: WindowID) {
        controller.id = id
        controller.view.updateID(id: id)
    }

    public func setTitle(title: String) {
        if let window = self.controller.window {
            window.title = title
        }
    }

    public func drawBitmap(data: UnsafeMutablePointer<CUnsignedChar>) {
        controller.drawBitmap(data: data)
    }

    public func nativeHandle() -> UnsafeMutableRawPointer {
        return toCPtr(controller.view.layer)
    }

    public func nativeHandle2() -> UnsafeMutableRawPointer {
        return withUnsafeMutablePointer(to: &(controller.view.layer)) { ptr in
            return UnsafeMutableRawPointer(ptr)
        }
    }
}
