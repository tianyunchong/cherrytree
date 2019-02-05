#include "ct_dialogs.h"
#include <gtkmm/stock.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <gtkmm/cellrendererpixbuf.h>
#include <gtkmm/colorchooserdialog.h>
#include "ct_app.h"
#include "ct_treestore.h"

using namespace ct_dialogs;

Glib::RefPtr<CtChooseDialogListStore> CtChooseDialogListStore::create()
{
    Glib::RefPtr<CtChooseDialogListStore> model(new CtChooseDialogListStore());
    model->set_column_types(model->columns);
    return model;
}

void CtChooseDialogListStore::add_row(const std::string& stock_id, const std::string& key, const std::string& desc)
{
    auto row = *append();
    row[columns.stock_id] = stock_id;
    row[columns.key] = key;
    row[columns.desc] = desc;
}


Gtk::TreeModel::iterator ct_dialogs::choose_item_dialog(Gtk::Window& parent, const std::string& title,
                                                        Glib::RefPtr<CtChooseDialogListStore> model,
                                                        const gchar* one_columns_name /* = nullptr */)
{
    Gtk::Dialog dialog(title, parent, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.set_transient_for(parent);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(400, 300);
    Gtk::ScrolledWindow* scrolledwindow = Gtk::manage(new Gtk::ScrolledWindow());
    scrolledwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    Gtk::TreeView* elements_treeview = Gtk::manage(new Gtk::TreeView(model));
    elements_treeview->set_headers_visible(false);
    Gtk::CellRendererPixbuf pix_buf_renderer;
    if (one_columns_name == nullptr) {
        int col_num = elements_treeview->append_column("", pix_buf_renderer) - 1;
        elements_treeview->get_column(col_num)->add_attribute(pix_buf_renderer, "icon-name", model->columns.stock_id);
    }
    elements_treeview->append_column(one_columns_name ? one_columns_name : "", model->columns.desc);
    scrolledwindow->add(*elements_treeview);
    //list_parms->sel_iter = elements_liststore->get_iter_first()
    //if list_parms->sel_iter:
    //    elements_treeview->set_cursor(elements_liststore->get_path(list_parms->sel_iter))
    auto content_area = dialog.get_content_area();
    content_area->pack_start(*scrolledwindow);
    content_area->show_all();
    elements_treeview->grab_focus();

    if (dialog.run() != Gtk::RESPONSE_ACCEPT) return Gtk::TreeModel::iterator();
    return elements_treeview->get_selection()->get_selected();
}

// Dialog to select a color, featuring a palette
bool ct_dialogs::color_pick_dialog(Gtk::Window& parent, Gdk::RGBA& color)
{
    Gtk::ColorChooserDialog dialog(_("Pick a Color"), parent);
    dialog.set_transient_for(parent);
    dialog.set_modal(true);
    dialog.set_property("destroy-with-parent", true);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    std::vector<std::string> colors = str::split(CtApp::P_ctCfg->colorPalette, ":");
    std::vector<Gdk::RGBA> rgbas;
    for (const auto& c: colors)
        rgbas.push_back(Gdk::RGBA(c));
    dialog.add_palette(Gtk::Orientation::ORIENTATION_HORIZONTAL, 10, rgbas);
    dialog.set_rgba(color);

    if (Gtk::RESPONSE_OK != dialog.run())
        return false;

    std::string ret_color_hex8 = CtRgbUtil::rgb_any_to_24(dialog.get_rgba());
    size_t color_qty = colors.size();
    colors.erase(std::find(colors.begin(), colors.end(), ret_color_hex8));
    if (color_qty == colors.size())
        colors.pop_back();
    colors.insert(colors.begin(), ret_color_hex8);

    color = dialog.get_rgba();
    return true;
}

// The Question dialog, returns True if the user presses OK
bool ct_dialogs::question_dialog(const std::string& message, Gtk::Window& parent)
{
    Gtk::MessageDialog dialog(parent, _("Question"),
              true /* use_markup */, Gtk::MESSAGE_QUESTION,
              Gtk::BUTTONS_OK_CANCEL, true /* modal */);
    dialog.set_secondary_text(message);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    return dialog.run() == Gtk::RESPONSE_OK;
}

// The Info dialog
void ct_dialogs::info_dialog(const std::string& message, Gtk::Window& parent)
{
    Gtk::MessageDialog dialog(parent, _("Info"),
              true /* use_markup */, Gtk::MESSAGE_INFO,
              Gtk::BUTTONS_OK, true /* modal */);
    dialog.set_secondary_text(message);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.run();
}

// The Warning dialog
void ct_dialogs::warning_dialog(const std::string& message, Gtk::Window& parent)
{
    Gtk::MessageDialog dialog(parent, _("Warning"),
              true /* use_markup */, Gtk::MESSAGE_WARNING,
              Gtk::BUTTONS_OK, true /* modal */);
    dialog.set_secondary_text(message);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.run();
}

// The Error dialog
void ct_dialogs::error_dialog(const std::string& message, Gtk::Window& parent)
{
    Gtk::MessageDialog dialog(parent, _("Error"),
              true /* use_markup */, Gtk::MESSAGE_ERROR,
              Gtk::BUTTONS_OK, true /* modal */);
    dialog.set_secondary_text(message);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.run();
}

// Dialog to Select a Node
Gtk::TreeIter ct_dialogs::choose_node_dialog(Gtk::Window& parent, Gtk::TreeView& parentTreeView, const std::string& title, CtTreeStore* treestore, Gtk::TreeIter sel_tree_iter)
{
    Gtk::Dialog dialog(title, parent, Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_DESTROY_WITH_PARENT);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
    dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
    dialog.set_position(Gtk::WindowPosition::WIN_POS_CENTER_ON_PARENT);
    dialog.set_default_size(600, 500);
    auto treeview_2 = Gtk::TreeView(treestore->get_store());
    treeview_2.set_headers_visible(false);
    treeview_2.set_search_column(1);
    treeview_2.append_column("", treestore->get_columns().rColPixbuf);
    treeview_2.append_column("", treestore->get_columns().colNodeName);
    auto scrolledwindow = Gtk::ScrolledWindow();
    scrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolledwindow.add(treeview_2);
    auto content_area = dialog.get_content_area();
    content_area->pack_start(scrolledwindow);

    auto expand_collapse_row = [&treeview_2](Gtk::TreePath path) {
        if (treeview_2.row_expanded(path))
            treeview_2.collapse_row(path);
        else
            treeview_2.expand_row(path, false);
    };

    treeview_2.signal_event().connect([&treeview_2, &expand_collapse_row](GdkEvent* event)->bool{
        if (event->type != GDK_BUTTON_PRESS && event->type!= GDK_2BUTTON_PRESS && event->type != GDK_KEY_PRESS)
            return false;
        if (event->type == GDK_BUTTON_PRESS && event->button.button == 2) {
            Gtk::TreePath path_at_click;
            if (treeview_2.get_path_at_pos(event->button.x, event->button.y, path_at_click)) {
                expand_collapse_row(path_at_click);
                return true;
            }
        } else if (event->type == GDK_2BUTTON_PRESS && event->button.button == 1) {
            if (treeview_2.get_selection()->get_selected()) {
                expand_collapse_row(treeview_2.get_model()->get_path(treeview_2.get_selection()->get_selected()));
                return true;
            }
        } else if (event->type == GDK_KEY_PRESS && treeview_2.get_selection()->get_selected()) {
            if (event->key.keyval == GDK_KEY_Left)
                treeview_2.collapse_row(treeview_2.get_model()->get_path(treeview_2.get_selection()->get_selected()));
            else if (event->key.keyval == GDK_KEY_Right)
                treeview_2.expand_row(treeview_2.get_model()->get_path(treeview_2.get_selection()->get_selected()), false);
            else
                return false;
            return true;
        }
        return false;
    });
    
    content_area->show_all();
    std::string expanded_collapsed_string = treestore->get_tree_expanded_collapsed_string(parentTreeView);
    treestore->set_tree_expanded_collapsed_string(expanded_collapsed_string, treeview_2, CtApp::P_ctCfg->nodesBookmExp);
    if (sel_tree_iter) {
        Gtk::TreePath sel_path = treeview_2.get_model()->get_path(sel_tree_iter);
        treeview_2.expand_to_path(sel_path);
        treeview_2.set_cursor(sel_path);
        treeview_2.scroll_to_row(sel_path);
    }

    return dialog.run() == Gtk::RESPONSE_ACCEPT ? treeview_2.get_selection()->get_selected() : Gtk::TreeIter();
}
