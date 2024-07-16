#include "scheme_url_loader.h"

#include "nope.h"

#include "base/task/thread_pool.h"
#include "content/public/browser/browser_thread.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/self_deleting_url_loader_factory.h"
#include "services/network/public/mojom/url_loader.mojom.h"
#include "services/network/public/mojom/url_response_head.mojom.h"

namespace {
    class Factory final : public network::SelfDeletingURLLoaderFactory {
        void CreateLoaderAndStart(
            mojo::PendingReceiver<network::mojom::URLLoader> loader,
            int32_t request_id,
            uint32_t options,
            network::ResourceRequest const& request,
            mojo::PendingRemote<network::mojom::URLLoaderClient> client,
            net::MutableNetworkTrafficAnnotationTag const& traffic_annotation
            ) override;
        raw_ptr<network::mojom::URLLoaderFactory> lower_;
    public:
        Factory(mojo::PendingReceiver<network::mojom::URLLoaderFactory>,network::mojom::URLLoaderFactory*);
    };
}

void electron_spin::CreateLoaderFactories(
    SchemeToLoaderMap* in_out,
    content::BrowserContext* context,
    network::mojom::URLLoaderFactory* default_factory,
    network::mojom::NetworkContext* net_ctxt,
    PrefService* pref_svc) {
    LOG(ERROR) << "spun with " << in_out->size() << " schemes.";
    mojo::PendingRemote<network::mojom::URLLoaderFactory> pending;
    new Factory(pending.InitWithNewPipeAndPassReceiver(), default_factory);
    in_out->insert_or_assign("https", std::move(pending));
}

namespace {
    class Loader : public network::mojom::URLLoader {
        mojo::Remote<network::mojom::URLLoaderClient> client_;
        GURL from_url_;

        void FollowRedirect(
            std::vector<std::string> const& removed_headers,
            net::HttpRequestHeaders const& modified_headers,
            net::HttpRequestHeaders const& modified_cors_exempt_headers,
            std::optional<::GURL> const& new_url) override {}
        void SetPriority(net::RequestPriority priority,
                        int32_t intra_priority_value) override {}
        void PauseReadingBodyFromNet() override {}
        void ResumeReadingBodyFromNet() override {}

    public:
        Loader(mojo::PendingRemote<network::mojom::URLLoaderClient>,GURL);
        void Respond();
    };
    Factory::Factory(mojo::PendingReceiver<network::mojom::URLLoaderFactory> r,network::mojom::URLLoaderFactory*l)
    : network::SelfDeletingURLLoaderFactory(std::move(r))
    , lower_{l}
    {
        LOG(ERROR) << "Factory ctor";
    }
    void Factory::CreateLoaderAndStart(
        mojo::PendingReceiver<network::mojom::URLLoader> loader,
        int32_t request_id,
        uint32_t options,
        network::ResourceRequest const& r,
        mojo::PendingRemote<network::mojom::URLLoaderClient> client,
        net::MutableNetworkTrafficAnnotationTag const& traffic_annotation) {
        if (r.url.has_path() && r.url.path_piece().ends_with(".jpg")) {
            auto l = std::make_shared<Loader>(std::move(client), r.url);
            content::GetUIThreadTaskRunner()->PostTask(FROM_HERE,
            // content::GetIOThreadTaskRunner()->PostTask(FROM_HERE,
            // base::PostTask(FROM_HERE, {content::BrowserThread::UI},
            // base::ThreadPool::PostTask(FROM_HERE, {},
            // base::ThreadPool::PostTask(FROM_HERE,
                           base::BindOnce(&Loader::Respond, l));
        } else {
            lower_->CreateLoaderAndStart(
                std::move(loader),
                request_id,
                options,
                r,
                std::move(client),
                traffic_annotation);
        }
    }

    Loader::Loader(mojo::PendingRemote<network::mojom::URLLoaderClient> c,GURL u)
    : client_{std::move(c)}
    , from_url_{u}
    {
        LOG(ERROR) << "Loader ctor " << u.spec();
    }
    void Loader::Respond() {
        mojo::ScopedDataPipeProducerHandle pipe_prod_ = {};
        mojo::ScopedDataPipeConsumerHandle pipe_cons_ = {};
        mojo::CreateDataPipe(nope_jpg_len, pipe_prod_, pipe_cons_);
        auto head = network::mojom::URLResponseHead::New();
        /*
        auto ri = net::RedirectInfo::ComputeRedirectInfo(
            "GET", GURL{from_url_}, net::SiteForCookies{},
            net::RedirectInfo::FirstPartyURLPolicy::UPDATE_URL_ON_REDIRECT,
            net::ReferrerPolicy::NO_REFERRER, "", 301,
            // GURL{"data:image/jpeg;base64,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"},
            // GURL{"file:///home/lbl/Pictures/nope.jpg"},
            GURL{"https://friconix.com/png/fi-ctluxs-smiley-happy-thin.png"},
            std::nullopt, false);
        client_->OnReceiveRedirect(ri, std::move(head));
        */
        auto len = nope_jpg_len;
        pipe_prod_->WriteData(nope_jpg, &len, MOJO_BEGIN_WRITE_DATA_FLAG_ALL_OR_NONE);
        client_->OnReceiveResponse(std::move(head), std::move(pipe_cons_), absl::nullopt);
        client_->OnComplete(network::URLLoaderCompletionStatus{});
    }
}
