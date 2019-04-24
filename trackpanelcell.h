/**********************************************************************

Audacity: A Digital Audio Editor

TrackPanelCell.h

Paul Licameli

**********************************************************************/

#ifndef __AUDACITY_TRACK_PANEL_CELL__
#define __AUDACITY_TRACK_PANEL_CELL__

//#include "MemoryX.h"
#include "memoryx.h"

#include <vector>

namespace RF {
    class AudacityProject;
    struct HitTestPreview;
    struct TrackPanelMouseEvent;
    struct TrackPanelMouseState;
    class ViewInfo;
    class wxKeyEvent;
    class wxPoint;
    class wxRect;
    class wxWindow;

    class UIHandle;
    using UIHandlePtr = std::shared_ptr<UIHandle>;


    /// \brief The TrackPanel is built up of nodes, subtrees of the CellularPanel's area
    /// This class itself has almost nothing in it.  Other classes derived from it
    /// build up the capabilities.
    class /*AUDACITY_DLL_API*/ /* not final */ TrackPanelNode
    {
    public:
        TrackPanelNode();
        virtual ~TrackPanelNode() = 0;
    };

    // A node of the TrackPanel that contins other nodes.
    class /*AUDACITY_DLL_API*/ TrackPanelGroup /* not final */ : public TrackPanelNode
    {
    public:
        TrackPanelGroup();
        virtual ~TrackPanelGroup();

        enum class Axis { X, Y };

        // A refinement of a given rectangle partitions it along one of its axes
        // and associates TrackPanelNodes with the partition.
        // The sequence of coordinates should be increasing, giving left or top
        // coordinates of sub-rectangles.
        // Null pointers are permitted to define empty spaces with no cell object.
        // If the first coordinate is right of or below the rectangle boundary,
        // then that also defines an empty space at the edge.
        // Sub-rectangles may be defined partly or wholly out of the bounds of the
        // given rectangle.  Such portions are ignored.
        using Child = std::pair< int, std::shared_ptr<TrackPanelNode> >;
        using Refinement = std::vector< Child >;
        using Subdivision = std::pair< Axis, Refinement >;

        // Report a subdivision of one of the axes of the given rectangle
        virtual Subdivision Children( const wxRect &rect ) = 0;
    };

    /// Abstract base class defining TrackPanel's access to specialist classes that
    /// implement drawing and user interactions
    /*
抽象基类，定义TrackPanel对实现绘图和用户交互的专业类的访问
*/
    class /*AUDACITY_DLL_API*/ TrackPanelCell /* not final */ : public TrackPanelNode
    {
    public:
        virtual ~TrackPanelCell () = 0;

        // May supply default cursor, status message, and tooltip, when there is no
        // handle to hit at the mouse position, or the handle does not supply them.
        // 当没有手柄敲击鼠标位置或手柄不提供它们时，可以提供默认光标，
        //  状态消息和工具提示
        virtual HitTestPreview DefaultPreview
        (const TrackPanelMouseState &state, const AudacityProject *pProject);

        // Return pointers to objects that can be queried for a status
        // bar message and cursor appropriate to the point, and that dispatch
        // mouse button events.
        // The button-down state passed to the function is as it will be at click
        // time -- not necessarily as it is now.
        /*
    返回指向对象的指针，可以查询状态栏消息和适合该点的光标，
    以及调度鼠标按钮事件。
    传递给函数的按钮状态与点击时一样 - 不一定就像现在一样。
    */
        virtual std::vector<UIHandlePtr> HitTest
        (const TrackPanelMouseState &state,
         const AudacityProject *pProject) = 0;

        // Return value is a bitwise OR of RefreshCode values
        // Include Cancelled in the flags to indicate that the event is not handled.
        // Default does only that.
        /*
   返回值是RefreshCode值的按位OR，包含在标志中已取消，表示未处理事件。
默认只做那个
   */
        virtual unsigned HandleWheelRotation
        (const TrackPanelMouseEvent &event,
         AudacityProject *pProject);

        // A cell may delegate context menu handling to another one
        // 单元格可以将上下文菜单处理委托给另一个
        virtual std::shared_ptr<TrackPanelCell> ContextMenuDelegate()
        { return {}; }

        // The pPosition parameter indicates mouse position but may be NULL
        // Return value is a bitwise OR of RefreshCode values
        // Default implementation does nothing
        virtual unsigned DoContextMenu
        (const wxRect &rect,
         wxWindow *pParent, wxPoint *pPosition);

        // Return value is a bitwise OR of RefreshCode values
        // Default skips the event and does nothing
        /*
   pPosition参数指示鼠标位置但可能为NULL返回值是RefreshCode
   值的按位OR默认实现不执行任何操作
   */
        virtual unsigned CaptureKey
        (wxKeyEvent &event, ViewInfo &viewInfo, wxWindow *pParent);

        // Return value is a bitwise OR of RefreshCode values
        // Default skips the event and does nothing
        /*
   返回值是RefreshCode值的按位OR。默认值会跳过事件并且不执行任何操作
   */
        virtual unsigned KeyDown
        (wxKeyEvent & event, ViewInfo &viewInfo, wxWindow *pParent);

        // Return value is a bitwise OR of RefreshCode values
        // Default skips the event and does nothing
        /*
   返回值是RefreshCode值的按位OR
默认会跳过该事件并且不执行任何操作
   */
        virtual unsigned KeyUp
        (wxKeyEvent & event, ViewInfo &viewInfo, wxWindow *pParent);

        // Return value is a bitwise OR of RefreshCode values
        // Default skips the event and does nothing
        /*
   返回值是RefreshCode值的按位OR
默认会跳过该事件并且不执行任何操作
   */
        virtual unsigned Char
        (wxKeyEvent & event, ViewInfo &viewInfo, wxWindow *pParent);
    };
}

#endif
