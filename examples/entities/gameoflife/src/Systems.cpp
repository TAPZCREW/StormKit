#ifdef STORMKIT_BUILD_MODULES
module Systems;

import Components;
#else
    #include <stormkit/std.hpp>

    #include <stormkit.core.hpp>

    #include "Components.mpp"
    #include "Systems.mpp"
#endif

using namespace stormkit;

UpdateBoardSystem::UpdateBoardSystem(stormkit::image::Image&  board,
                                     Renderer&                renderer,
                                     entities::EntityManager& manager)
    : entities::System { manager, 0, { PositionComponent::TYPE } }, m_board { &board },
      m_renderer { &renderer }, m_last_update { Clock::now() } {
}

UpdateBoardSystem::~UpdateBoardSystem()                                               = default;
UpdateBoardSystem::UpdateBoardSystem(UpdateBoardSystem&&) noexcept                    = default;
auto UpdateBoardSystem::operator=(UpdateBoardSystem&&) noexcept -> UpdateBoardSystem& = default;

auto UpdateBoardSystem::update(stormkit::Secondf delta) -> void {
    const auto now = Clock::now();

    if (m_is_on_edit_mode) [[unlikely]] {
        m_updated = true;
        return;
    }

    if (now - m_last_update < m_refresh_board_delta) [[likely]]
        return;

    m_last_update = now;

    m_updated = true;

    struct Cell {
        UInt32 x;
        UInt32 y;

        bool alive = false;

        entities::Entity e = entities::INVALID_ENTITY;

        UInt32 adjacent_alive_cells = 0;
    };

    constexpr auto CELL_COUNT = BOARD_SIZE * BOARD_SIZE;

    auto cell_status = transform(range(CELL_COUNT), [](const auto i) {
        return Cell { i % BOARD_SIZE, i / BOARD_SIZE };
    });

    for (const auto e : m_entities) {
        const auto& position = m_manager->getComponent<PositionComponent>(e);

        auto it
            = std::ranges::find_if(cell_status, [x = position.x, y = position.y](const auto& cell) {
                  return cell.x == x && cell.y == y;
              });

        if (it != std::ranges::cend(cell_status)) {
            it->alive = true;
            it->e     = e;

            const auto x = position.x;
            const auto y = position.y;

            if (const auto id = (x + 1) + BOARD_SIZE * y; id < CELL_COUNT && (x + 1) < BOARD_SIZE)
                cell_status[id].adjacent_alive_cells += 1;
            if (const auto id = (x - 1) + BOARD_SIZE * y; id < CELL_COUNT && (x - 1) < BOARD_SIZE)
                cell_status[id].adjacent_alive_cells += 1;
            if (const auto id = x + BOARD_SIZE * (y - 1); id < CELL_COUNT && (y - 1) < BOARD_SIZE)
                cell_status[id].adjacent_alive_cells += 1;
            if (const auto id = x + BOARD_SIZE * (y + 1); id < CELL_COUNT && (y + 1) < BOARD_SIZE)
                cell_status[id].adjacent_alive_cells += 1;
            if (const auto id = (x - 1) + BOARD_SIZE * (y - 1);
                id < CELL_COUNT && (x - 1) < BOARD_SIZE && (y - 1) < BOARD_SIZE)
                cell_status[id].adjacent_alive_cells += 1;
            if (const auto id = (x - 1) + BOARD_SIZE * (y + 1);
                id < CELL_COUNT && (x - 1) < BOARD_SIZE && (y + 1) < BOARD_SIZE)
                cell_status[id].adjacent_alive_cells += 1;
            if (const auto id = (x + 1) + BOARD_SIZE * (y - 1);
                id < CELL_COUNT && (x + 1) < BOARD_SIZE && (y - 1) < BOARD_SIZE)
                cell_status[id].adjacent_alive_cells += 1;
            if (const auto id = (x + 1) + BOARD_SIZE * (y + 1);
                id < CELL_COUNT && (x + 1) < BOARD_SIZE && (y + 1) < BOARD_SIZE)
                cell_status[id].adjacent_alive_cells += 1;
        }
    }

    for (const auto& cell : cell_status) {
        const auto alive
            = cell.adjacent_alive_cells == 3 || (cell.alive && cell.adjacent_alive_cells == 2);

        if (alive && !m_manager->hasEntity(cell.e)) {
            auto  e        = m_manager->makeEntity();
            auto& position = m_manager->addComponent<PositionComponent>(e);
            position.x     = cell.x;
            position.y     = cell.y;
        } else if (!alive && m_manager->hasEntity(cell.e))
            m_manager->destroyEntity(cell.e);
    }
    /*
        for (const auto &cell : cell_status) {
            if (m_entities.contains(cell.e)) {
                if (!cell.alive) {
                    m_manager->destroyEntity(cell.e);

                    const auto x = cell.x;
                    const auto y = cell.y;

                    if (const auto id = (x + 1) + BOARD_SIZE * y;
                        id < CELL_COUNT && (x - 1) < BOARD_SIZE)
                        cell_status[id].adjacent_alive_cells -= 1;
                    if (const auto id = (x - 1) + BOARD_SIZE * y;
                        id < CELL_COUNT && (x - 1) < BOARD_SIZE)
                        cell_status[id].adjacent_alive_cells -= 1;
                    if (const auto id = x + BOARD_SIZE * (y - 1);
                        id < CELL_COUNT && (y - 1) < BOARD_SIZE)
                        cell_status[id].adjacent_alive_cells -= 1;
                    if (const auto id = x + BOARD_SIZE * (y + 1);
                        id < CELL_COUNT && (y + 1) < BOARD_SIZE)
                        cell_status[id].adjacent_alive_cells -= 1;
                }
            }
        }

        for (auto &cell : cell_status) {
            if (cell.adjacent_alive_cells == 3) {
                cell.alive = true;

                auto e         = m_manager->makeEntity();
                auto &position = m_manager->addComponent<PositionComponent>(e);
                position.x     = cell.x;
                position.y     = cell.y;
            }
        }*/
}

auto UpdateBoardSystem::postUpdate() -> void {
    using namespace stormkit::literals;
    if (m_updated) {
        m_updated          = false;
        auto&       board  = *m_board;
        const auto& extent = board.extent();

        for (auto i : range(extent.width * extent.height)) {
            auto pixel = board.pixel(i);
            pixel[0]   = 0_b;
            pixel[1]   = 0_b;
            pixel[2]   = 0_b;
            pixel[3]   = 255_b;
        }

        for (const auto e : m_entities) {
            const auto& position = m_manager->getComponent<PositionComponent>(e);

            auto pixel = board.pixel({ position.x, position.y, 0 });
            pixel[0]   = 255_b;
            pixel[1]   = 255_b;
            pixel[2]   = 255_b;
            pixel[3]   = 255_b;
        }

        m_renderer->updateBoard(board);
    }
}
