print('Loading Redstone...')

define_behavior('push_move', MESSAGE_PUSH + MESSAGE_PULL, function()
    print('Running push_move')
end)

define_behavior('push_break', MESSAGE_PUSH, function()
    print('Running push_break')
end)

define_behavior('power_wire', MESSAGE_POWER, function()
    print('Running power_wire')
end)

define_behavior('power_conductor', MESSAGE_POWER, function()
    print('Running power_conductor')
end)

define_behavior('power_torch', MESSAGE_POWER, function()
    print('Running power_torch')
end)

define_behavior('power_piston', MESSAGE_POWER, function()
    print('Running power_piston')
end)

define_behavior('power_repeater', MESSAGE_POWER, function()
    print('Running power_repeater')
end)

define_behavior('power_comparator', MESSAGE_POWER, function()
    print('Running power_comparator')
end)

define_behavior('power_switch', MESSAGE_POWER, function()
    print('Running power_switch')
end)

define_type('air',        0)
define_type('insulator',  0, 'push_move')
define_type('wire',       1, 'push_break', 'power_wire')
define_type('conductor',  1, 'push_move',  'power_conductor')
define_type('torch',      2, 'push_break', 'power_torch')
define_type('piston',     2, 'push_break', 'power_piston')
define_type('repeater',   3, 'push_break', 'power_repeater')
define_type('comparator', 3, 'push_break', 'power_comparator')
define_type('switch',     3, 'push_break', 'power_switch')

