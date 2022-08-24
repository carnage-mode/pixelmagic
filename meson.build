project('pixelmagic', ['cpp', 'c'],
          version: '0.1.0',
    meson_version: '>= 0.59.0',
  default_options: [ 'warning_level=3',
                     'buildtype=debug',
                     'werror=true',
                     'cpp_std=gnu++20',
                     'cpp_args=-Wall -Weffc++ -Wextra -Wsign-conversion -pedantic-errors',
                     'b_sanitize=leak',
                     'b_sanitize=thread',
                     'b_sanitize=memory',
                     'b_sanitize=address,undefined',
                   ],
)





subdir('src')

