package com.framework.widgets.jakerecycleview;

import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.StaggeredGridLayoutManager;
import android.view.View;
import android.view.ViewGroup;

/**
 * 具有Header、Footer的Adapter
 */
class WrapperAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {
    protected static final int HEADER = Integer.MIN_VALUE;
    protected static final int FOOTER = Integer.MAX_VALUE;

    private final RecyclerView.Adapter mAdapter;

    private View mHeaderItemView;

    private View mFooterItemView;
    private RecyclerView.AdapterDataObserver mObserver = new RecyclerView.AdapterDataObserver() {
        @Override
        public void onChanged() {
            WrapperAdapter.this.notifyDataSetChanged();
        }

        @Override
        public void onItemRangeChanged(int positionStart, int itemCount) {
            WrapperAdapter.this.notifyItemRangeChanged(positionStart + 1, itemCount);
        }

        @Override
        public void onItemRangeChanged(int positionStart, int itemCount, Object payload) {
            WrapperAdapter.this.notifyItemRangeChanged(positionStart + 1, itemCount, payload);
        }

        @Override
        public void onItemRangeInserted(int positionStart, int itemCount) {
            WrapperAdapter.this.notifyItemRangeInserted(positionStart + 1, itemCount);
        }

        @Override
        public void onItemRangeRemoved(int positionStart, int itemCount) {
            WrapperAdapter.this.notifyItemRangeRemoved(positionStart + 1, itemCount);
        }

        @Override
        public void onItemRangeMoved(int fromPosition, int toPosition, int itemCount) {
            WrapperAdapter.this.notifyDataSetChanged();
        }
    };

    public WrapperAdapter(RecyclerView.Adapter adapter, View headerContainer, View footerContainer) {
        this.mAdapter = adapter;
        this.mHeaderItemView = headerContainer;
        this.mFooterItemView = footerContainer;
        mAdapter.registerAdapterDataObserver(mObserver);
    }

    public RecyclerView.Adapter getAdapter() {
        return mAdapter;
    }

    @Override
    public void onAttachedToRecyclerView(final RecyclerView recyclerView) {
        RecyclerView.LayoutManager layoutManager = recyclerView.getLayoutManager();
        if (layoutManager instanceof GridLayoutManager) {
            final GridLayoutManager gridLayoutManager = (GridLayoutManager) layoutManager;
            gridLayoutManager.setSpanSizeLookup(new GridLayoutManager.SpanSizeLookup() {
                @Override
                public int getSpanSize(int position) {
                    WrapperAdapter wrapperAdapter = (WrapperAdapter) recyclerView.getAdapter();
                    if (isFullSpanType(wrapperAdapter.getItemViewType(position))) {
                        return gridLayoutManager.getSpanCount();
                    }
                    return 1;

                }
            });
        }
    }

    @Override
    public void onViewAttachedToWindow(RecyclerView.ViewHolder holder) {
        super.onViewAttachedToWindow(holder);
        int position = holder.getLayoutPosition();
        int type = getItemViewType(position);
        if (isFullSpanType(type)) {
            ViewGroup.LayoutParams layoutParams = holder.itemView.getLayoutParams();
            if (layoutParams instanceof StaggeredGridLayoutManager.LayoutParams) {
                StaggeredGridLayoutManager.LayoutParams lp = (StaggeredGridLayoutManager.LayoutParams) layoutParams;
                lp.setFullSpan(true);
            }
        }
    }

    private boolean isFullSpanType(int type) {
        return type == HEADER || type == FOOTER;
    }

    @Override
    public int getItemCount() {
        return mAdapter.getItemCount() + 2;
    }

    @Override
    public int getItemViewType(int position) {
        if (position == 0) {
            return HEADER;
        } else if (0 < position && position < mAdapter.getItemCount() + 1) {
            return mAdapter.getItemViewType(position - 1);
        } else if (position == mAdapter.getItemCount() + 1) {
            return FOOTER;
        } else {
            throw new IllegalArgumentException("Wrong type! Position = " + position);
        }
    }

    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        if (viewType == HEADER) {
            return new HeaderContainerViewHolder(mHeaderItemView);
        } else if (viewType == FOOTER) {
            return new FooterContainerViewHolder(mFooterItemView);
        } else {
            return mAdapter.onCreateViewHolder(parent, viewType);
        }
    }

    @Override
    public void onBindViewHolder(RecyclerView.ViewHolder holder, int position) {
        if (0 < position && position < mAdapter.getItemCount() + 1) {
            mAdapter.onBindViewHolder(holder, position - 1);
        }
    }


    static class HeaderContainerViewHolder extends RecyclerView.ViewHolder {

        public HeaderContainerViewHolder(View itemView) {
            super(itemView);
        }
    }

    static class FooterContainerViewHolder extends RecyclerView.ViewHolder {

        public FooterContainerViewHolder(View itemView) {
            super(itemView);
        }
    }
}
