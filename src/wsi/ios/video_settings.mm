#include <storm/window/VideoSettings.hpp>

#import <UIKit/UIScreen.h>

using namespace storm;
using namespace storm::window;

std::vector<VideoSettings> VideoSettings::getDesktopModes() {
    const auto screen_rect  = [UIScreen mainScreen].bounds;
    const auto scale_factor = [UIScreen mainScreen].nativeScale;

    const auto video_setting = VideoSettings {
        .size = { .width  = static_cast<core::u16>(screen_rect.size.width * scale_factor),
                 .height = static_cast<core::u16>(screen_rect.size.height * scale_factor) }
    };

    return { std::move(video_setting) };
}
