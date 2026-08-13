# Snake Game - 贪吃蛇游戏

## 项目简介

基于 **raylib 5.5.0** 图形库实现的贪吃蛇游戏（P6 完整版）。使用 **C 语言** 编写主程序，底层数据结构为**带头双向循环链表**（来自 [HeaderedDoublyCircularList](./HeaderedDoublyCircularList/) 项目），支持 O(1) 的头插尾删操作。

## 游戏特性

- **标题画面**：中文"贪吃蛇" + 最高分显示，点击或按 Enter/Space 开始
- **四角星芒食物**：七彩八角星芒，颜色由坐标决定（稳定不闪烁）
- **蛇身随食物变色**：吃到什么颜色食物，蛇身就变什么颜色；蛇头始终亮黄色
- **右侧 HUD**：实时显示分数、速度（格/秒）、档位、加速进度条
- **暂停菜单**：ESC 暂停，支持继续/重开/退出，兼容键盘和鼠标操作
- **死亡弹窗**：展示本局分数与历史最高分，新纪录自动提示
- **最高分本地存档**：exe 同目录 `highscore.txt`，带严格合法性校验
- **双通道输入**：方向键 + WASD，物理轮询，输入法不拦截
- **音效**：吃食物 `eat.wav`、死亡 `death.wav`

## 架构分层

| 层 | 文件 | 职责 |
|----|------|------|
| **适配层** | `snake_list.h / .c` | 把双向循环链表包装为蛇身结构 |
| **逻辑层** | `game_logic.h / .c` | 纯 C 逻辑：推进、转向、碰撞判定、吃食物、计分、最高分存档 |
| **配置层** | `config.h / ui_config.h` | 棋盘 30×30、速度 8→18、每 5 食物升档、颜色/字体/窗口 |
| **界面层** | `main.c` | raylib 渲染 + 物理键盘轮询 + 屏幕状态机 |

### 屏幕状态机

```
标题画面 → 游戏中 ↔ 暂停 → 死亡弹窗 → (重开) → 游戏中 ...
```

## 键盘操作

| 按键 | 功能 |
|------|------|
| ↑ / W | 向上 |
| ↓ / S | 向下 |
| ← / A | 向左 |
| → / D | 向右 |
| Enter / Space | 标题画面开始游戏 |
| ESC | 暂停 / 退出暂停 |

## 单元测试

`SnakeGameTests/` 包含 **49 个测试用例**（MSTest 框架）：

| 测试文件 | 覆盖内容 |
|----------|----------|
| `test_game_logic.cpp` | 23 个 GameLogic 用例 + 26 个 HighScore 用例 |
| `test_snake_list.cpp` | 9 个蛇身适配层用例 |

使用 Visual Studio Test Explorer 运行测试。

## 项目结构

```
snake game/
├── snake game.slnx
├── snake game/
│   ├── main.c                        # 游戏入口，含主循环
│   ├── game_logic.h / .c             # 游戏逻辑层
│   ├── snake_list.h / .c             # 蛇身适配层
│   ├── config.h                      # 全局配置
│   ├── ui_config.h                   # 界面配置
│   ├── snake game.vcxproj            # VS 项目配置
│   ├── snake game.vcxproj.filters
│   └── resources/
│       ├── simhei.ttf                # 中文字体
│       ├── eat.wav                   # 吃食物音效
│       └── death.wav                 # 死亡音效
└── SnakeGameTests/
    ├── SnakeGameTests.vcxproj
    ├── test_game_logic.cpp
    └── test_snake_list.cpp
```

## 依赖

| 依赖 | 版本 | 获取方式 |
|------|------|----------|
| raylib | 5.5.0 | NuGet 自动还原（`PackageReference`） |
| DCList（双向循环链表） | - | 引用 `..\..\HeaderedDoublyCircularList\HeaderedDoublyCircularList\` |

## 编译运行

使用 **Visual Studio 2022+**（MSVC v145，x64 平台）打开 `snake game.slnx` 即可自动还原 NuGet 依赖并编译运行。

> **注意**：依赖的 DCList 头文件和源文件需要与本项目在同一父目录下的 `HeaderedDoublyCircularList/` 中，否则需要调整 `vcxproj` 中的附加包含目录路径。
