#ifdef STORMKIT_BUILD_MODULES
module App;

import stormkit.Gpu;

import Components;
import Systems;
import Constants;
#else
    #include <stormkit/std.hpp>

    #include <stormkit.core.hpp>
    #include <stormkit/Gpu.hpp>

    #include "App.mpp"
    #include "Components.mpp"
    #include "Constants.mpp"
#endif

using namespace stormkit;

App::App() = default;

App::~App() {
    ilog("Cleaning");
}

auto App::run([[maybe_unused]] const int argc, [[maybe_unused]] const char** argv) -> Int32 {
    using Clock = std::chrono::high_resolution_clock;

    using namespace stormkit::literals;

    doInitWindow();

    m_board = image::Image {
        { BOARD_SIZE, BOARD_SIZE },
        image::Image::Format::RGBA8_UNORM
    };

    auto event_handler = wsi::EventHandler { *m_window };
    event_handler.addCallback(wsi::EventType::Closed,
                              [this]([[maybe_unused]]
                                     const wsi::Event& event) { m_window->close(); });
    event_handler.addCallback(wsi::EventType::KeyReleased, [this](const wsi::Event& event) {
        const auto& event_data = as<wsi::KeyReleasedEventData>(event.data);

        handleKeyboard(event_data);
    });
    event_handler.addCallback(wsi::EventType::MouseButtonPushed, [this](const wsi::Event& event) {
        const auto& event_data = as<wsi::MouseButtonPushedEventData>(event.data);

        handleMouse(event_data);
    });

    m_update_system = &m_entities.add_system<UpdateBoardSystem>(m_board, *m_renderer);

    for (auto i : range(m_board.extent().width * m_board.extent().height)) {
        auto pixel = m_board.pixel(i);
        pixel[0]   = 0_b;
        pixel[1]   = 0_b;
        pixel[2]   = 0_b;
        pixel[3]   = 255_b;
    }

    m_renderer->updateBoard(m_board);

    auto last_tp = Clock::now();
    while (m_window->isOpen()) {
        const auto now   = Clock::now();
        const auto delta = now - last_tp;
        last_tp          = now;

        event_handler.update();

        m_entities.step(delta);

        m_renderer->renderFrame();
    }

    return EXIT_SUCCESS;
}

auto App::doInitWindow() -> void {
    const auto window_style = wsi::WindowStyle::All;

    m_window = allocate<wsi::Window>(WINDOW_TITLE, math::ExtentU { 800u, 600u }, window_style);

    m_renderer = allocate<Renderer>(*m_window);
}

auto App::handleKeyboard(const stormkit::wsi::KeyReleasedEventData& event) -> void {
    using namespace stormkit::literals;

    const auto size = wsi::Window::getPrimaryMonitorSettings().sizes.back();

    switch (event.key) {
        [[unlikely]]
        case wsi::Key::Escape:
            m_window->close();
            break;
        [[unlikely]]
        case wsi::Key::F11:
            if (m_fullscreen_enabled) {
                m_fullscreen_enabled = false;
                m_window->setFullscreenEnabled(false);
            } else {
                m_fullscreen_enabled = true;
                m_window->setSize(size);
                m_window->setFullscreenEnabled(true);
            }
            break;
        case wsi::Key::R:
            for (auto i : range(m_board.extent().width * m_board.extent().height)) {
                auto pixel = m_board.pixel(i);
                pixel[0]   = 0_b;
                pixel[1]   = 0_b;
                pixel[2]   = 0_b;
                pixel[3]   = 255_b;
            }

            m_entities.destroy_all_entities();
            break;
        case wsi::Key::Space:
            m_is_on_edit_mode = !m_is_on_edit_mode;
            m_update_system->setEditModeEnabled(m_is_on_edit_mode);
            break;
        case wsi::Key::Add: m_update_system->incrementDelta(Secondf { 0.01f }); break;
        case wsi::Key::Substract: m_update_system->incrementDelta(Secondf { -0.01f }); break;
        default: break;
    }
}

auto App::handleMouse(const stormkit::wsi::MouseButtonPushedEventData& event) -> void {
    if (!m_is_on_edit_mode) [[likely]]
        return;
    if (event.button != wsi::MouseButton::Left) return;

    const auto cell_width  = as<float>(m_window->size().width) / as<float>(BOARD_SIZE);
    const auto cell_height = as<float>(m_window->size().height) / as<float>(BOARD_SIZE);

    const auto x = glm::floor(as<float>(event.position.x) / cell_width);
    const auto y = glm::floor(as<float>(event.position.y) / cell_height);

    const auto cells = m_entities.entities_with_component<PositionComponent>();
    const auto it    = std::ranges::find_if(cells, [&](const auto e) {
        const auto& position = m_entities.getComponent<PositionComponent>(e);

        return position.x == x && position.y == y;
    });

    if (it != std::ranges::cend(cells)) m_entities.destroy_entity(*it);
    else
        createCell(x, y);
}

auto App::createCell(stormkit::UInt32 x, stormkit::UInt32 y) -> void {
    auto  e        = m_entities.make_entity();
    auto& position = m_entities.add_component<PositionComponent>(e);

    position.x = x;
    position.y = y;
}
