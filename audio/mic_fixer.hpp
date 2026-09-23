#pragma once

namespace freshcore {
namespace mic_fixer {

// Starts a detached background thread that polls for active phone calls
// and applies the MicFix tinymix commands.
void StartPolling();

// Stops the polling thread if running.
void StopPolling();

} // namespace mic_fixer
} // namespace freshcore
